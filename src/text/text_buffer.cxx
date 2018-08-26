#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "opengl.h"

#include "text_buffer.h"
#include "program.h"

#include <iostream>
#include <vector>

namespace ftdgl {
namespace text {
namespace impl {
/*
  # 6x subpixel AA pattern
  #
  #   R = (f(x - 2/3, y) + f(x - 1/3, y) + f(x, y)) / 3
  #   G = (f(x - 1/3, y) + f(x, y) + f(x + 1/3, y)) / 3
  #   B = (f(x, y) + f(x + 1/3, y) + f(x + 2/3, y)) / 3
  #
  # The shader would require three texture lookups if the texture format
  # stored data for offsets -1/3, 0, and +1/3 since the shader also needs
  # data for offsets -2/3 and +2/3. To avoid this, the texture format stores
  # data for offsets 0, +1/3, and +2/3 instead. That way the shader can get
  # data for offsets -2/3 and -1/3 with only one additional texture lookup.
  #
*/

constexpr
glm::vec2 JITTER_PATTERN[] = {
    {-1 / 12.0, -5 / 12.0},
	{ 1 / 12.0,  1 / 12.0},
	{ 3 / 12.0, -1 / 12.0},
	{ 5 / 12.0,  5 / 12.0},
	{ 7 / 12.0, -3 / 12.0},
	{ 9 / 12.0,  3 / 12.0},
};

class TextBufferImpl : public TextBuffer {
public:
    TextBufferImpl(const viewport::viewport_s & viewport)
        : m_Viewport {viewport} {
        Init();
    }

    virtual ~TextBufferImpl() {
        Destroy();
    }

    virtual bool AddText(pen_s & pen, const markup_s & markup, const std::wstring & text);
    virtual uint32_t GetTexture() const { return m_RenderedTexture; }
    virtual void Clear();
    virtual uint32_t GetTextAttrCount() const { return m_TextAttribs.size(); }
    virtual const text_attr_s * GetTextAttr() const { return &m_TextAttribs[0]; }

private:
    bool AddChar(pen_s & pen,
                 const markup_s & markup,
                 const viewport::viewport_s & viewport,
                 wchar_t ch);

	GLuint m_RenderedTexture;
	GLuint m_FrameBuffer;
    const viewport::viewport_s & m_Viewport;
    double m_OriginX;

    ProgramPtr m_ProgramId;
    GLuint m_Position4Index;
    GLuint m_ColorIndex;
    GLuint m_Matrix4Index;

    std::vector<text_attr_s> m_TextAttribs;

private:
    void Init();
    void Destroy();
    void AddTextAttr(const pen_s & pen, const markup_s & markup,
                     const viewport::viewport_s & viewport);
};


void TextBufferImpl::AddTextAttr(const pen_s & pen, const markup_s & markup,
                                 const viewport::viewport_s & viewport) {
    auto adv_y = viewport.FontSizeToViewport(markup.font,
                                             markup.font->GetAscender() - markup.font->GetDescender(),
                                             false);

    m_TextAttribs.push_back(
        {
            {
                static_cast<float>(m_OriginX / viewport.window_width),
                static_cast<float>((pen.y - adv_y) / viewport.window_height),
                static_cast<float>(pen.x / viewport.window_width),
                static_cast<float>(pen.y / viewport.window_height)
            },

            {
                markup.fore_color.r,
                markup.fore_color.g,
                markup.fore_color.b,
                markup.fore_color.a
            },

            {
                markup.back_color.r,
                markup.back_color.g,
                markup.back_color.b,
                markup.back_color.a
            },
        });
}

bool TextBufferImpl::AddText(pen_s & pen, const markup_s & markup, const std::wstring & text) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    m_OriginX = pen.x;

    for(size_t i=0;i < text.length(); i++) {
        if (!AddChar(pen, markup, m_Viewport, text[i])) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }
    }

    if (pen.x != m_OriginX)
        AddTextAttr(pen, markup, m_Viewport);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool TextBufferImpl::AddChar(pen_s & pen,
                             const markup_s & markup,
                             const viewport::viewport_s & viewport,
                             wchar_t ch) {
    float pt_width = viewport.pixel_width * 72 / viewport.dpi;
    float pt_height = viewport.pixel_height * 72 / viewport.dpi_height;

    auto adv_y = viewport.FontSizeToViewport(markup.font,
                                             markup.font->GetAscender() - markup.font->GetDescender(),
                                             false);

    if (ch == L'\n') {
        if (pen.x != m_OriginX)
            AddTextAttr(pen, markup, viewport);

        pen.y -= adv_y;
        pen.x = m_OriginX;
        return true;
    }

    glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(-1, -1, 0));
    glm::mat4 translate1 = glm::translate(glm::mat4(1.0), glm::vec3(0, -markup.font->GetAscender(), 0));
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0), glm::vec3(2.0 * pen.x / viewport.window_width, 2.0 * pen.y / viewport.window_height, 0));
    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(1.0 / pt_width, 1.0 / pt_height, 0));

    glm::mat4 scale_font = glm::scale(glm::mat4(1.0), glm::vec3(markup.font->GetPtSize(), markup.font->GetPtSize(), 0));

    glm::mat4 transform = translate2 * translate * scale * scale_font * translate1;

    auto glyph = markup.font->LoadGlyph(ch);

    if (!glyph)
        return true;

    auto adv_x = viewport.FontSizeToViewport(markup.font, glyph->GetAdvanceX(), true);

    if (!glyph->NeedDraw()) {
        pen.x += adv_x;
        return true;
    }

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, glyph->GetSize(),
                 glyph->GetAddr(), GL_STATIC_DRAW);

    glUseProgram(*m_ProgramId);

    auto c = glm::vec4(0.0, 0.0, 0.0, 0.0);

    glEnableVertexAttribArray(m_Position4Index);
    glVertexAttribPointer(m_Position4Index, 4, GL_FLOAT, GL_FALSE, 0, 0);

    for(size_t i = 0; i < sizeof(JITTER_PATTERN) / sizeof(glm::vec2); i++) {
        glm::mat4 translate3 = glm::translate(glm::mat4(1.0), glm::vec3(JITTER_PATTERN[i].x * 72 / viewport.dpi / pt_width ,
                                                                        JITTER_PATTERN[i].y * 72 / viewport.dpi_height / pt_height,
                                                                        0));
        glm::mat4 transform_x = translate3 * transform;

        glUniformMatrix4fv(m_Matrix4Index,
                           1, GL_FALSE, &transform_x[0][0]);

        if (i % 2 == 0) {
            c = glm::vec4(i == 0 ? 1.0 : 0.0,
                          i == 2 ? 1.0 : 0.0,
                          i == 4 ? 1.0 : 0.0,
                          0.0);
        }

        glUniform4fv(m_ColorIndex, 1, &c[0]);

		glDrawArrays(GL_TRIANGLES, 0, glyph->GetSize() / sizeof(GLfloat) / 4);
    }

	glDeleteBuffers(1, &vertexbuffer);
    glDisableVertexAttribArray(m_Position4Index);
    glUseProgram(0);

    pen.x += adv_x;

    //TOOD: kerning
    return true;
}

void TextBufferImpl::Init() {
	glGenFramebuffers(1, &m_FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

	// The texture we're going to render to
	glGenTextures(1, &m_RenderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, m_RenderedTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, m_Viewport.pixel_width, m_Viewport.pixel_height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Viewport.pixel_width, m_Viewport.pixel_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderedTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("frame buffer status error\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
    glClearColor(0,0,0,0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_ProgramId = CreateTextBufferProgram();

    glUseProgram(*m_ProgramId);

    m_Matrix4Index = glGetUniformLocation(*m_ProgramId, "matrix4");
    m_ColorIndex = glGetUniformLocation(*m_ProgramId, "color");
    m_Position4Index = glGetAttribLocation(*m_ProgramId, "position4");

    glUseProgram(0);
}

void TextBufferImpl::Destroy() {
    glDeleteFramebuffers(1, &m_FrameBuffer);
    glDeleteTextures(1, &m_RenderedTexture);
}

void TextBufferImpl::Clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
    glClearColor(0,0,0,0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_TextAttribs.clear();
}

} //namespace impl

TextBufferPtr CreateTextBuffer(const viewport::viewport_s & viewport) {
    return std::make_shared<impl::TextBufferImpl>(viewport);
}

} //namespace text
} //namespace ftdgl
