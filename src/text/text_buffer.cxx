#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "opengl.h"

#include "text_buffer.h"
#include "program.h"
#include "char_width.h"

#include <iostream>
#include <vector>
#include <unordered_map>

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

typedef struct __matrix_color_s {
    glm::mat4 transform;
    glm::vec4 color;
} matrix_color_s;

typedef struct __draw_array_indirect_cmd_s{
    GLuint  count;
    GLuint  instanceCount;
    GLuint  first;
    GLuint  baseInstance;
} draw_array_indirect_cmd_s;

using matrix_color_vector = std::vector<matrix_color_s>;
using glyph_matrix_color_map = std::unordered_map<GlyphPtr, matrix_color_vector>;

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
    virtual void GenTexture();

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

    std::vector<text_attr_s> m_TextAttribs;
    glyph_matrix_color_map m_GlyphMatrixColors;

    bool m_TextureGenerated;
    uint32_t m_VertexCount;
private:
    void Init();
    void Destroy();
    void AddTextAttr(const pen_s & pen, const markup_s & markup,
                     const viewport::viewport_s & viewport);
};


void TextBufferImpl::AddTextAttr(const pen_s & pen, const markup_s & markup,
                                 const viewport::viewport_s & viewport) {
    auto adv_y = viewport.line_height ? viewport.line_height : viewport.FontSizeToViewport(markup.font,
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
    m_OriginX = pen.x;

    for(size_t i=0;i < text.length(); i++) {
        if (!AddChar(pen, markup, m_Viewport, text[i])) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }
    }

    if (pen.x != m_OriginX)
        AddTextAttr(pen, markup, m_Viewport);

    return true;
}

bool TextBufferImpl::AddChar(pen_s & pen,
                             const markup_s & markup,
                             const viewport::viewport_s & viewport,
                             wchar_t ch) {
    float pt_width = viewport.pixel_width * 72 / viewport.dpi;
    float pt_height = viewport.pixel_height * 72 / viewport.dpi_height;

    auto adv_y = viewport.line_height ? viewport.line_height : viewport.FontSizeToViewport(markup.font,
                                             markup.font->GetAscender() - markup.font->GetDescender(),
                                             false);

    if (ch == L'\n') {
        if (pen.x != m_OriginX)
            AddTextAttr(pen, markup, viewport);

        pen.y -= adv_y;
        pen.x = m_OriginX;
        return true;
    }

    auto glyph = markup.font->LoadGlyph(ch);

    if (!glyph)
        return true;

    auto glyph_adv_x = viewport.FontSizeToViewport(markup.font, glyph->GetAdvanceX(), true);
    auto adv_x = viewport.glyph_width ? (char_width(ch) > 1 ? viewport.glyph_width * 2 : viewport.glyph_width) : glyph_adv_x;

    if (!glyph->NeedDraw()) {
        pen.x += adv_x;
        return true;
    }

    m_TextureGenerated = false;

    glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(-1, -1, 0));
    glm::mat4 translate1 = glm::translate(glm::mat4(1.0), glm::vec3(0, -markup.font->GetAscender(), 0));
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0), glm::vec3(2.0 * pen.x / viewport.window_width, 2.0 * pen.y / viewport.window_height, 0));
    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(1.0 / pt_width, 1.0 / pt_height, 0));

    glm::mat4 scale_font = glm::scale(glm::mat4(1.0), glm::vec3(markup.font->GetPtSize(), markup.font->GetPtSize(), 0));

    glm::mat4 transform = translate2 * translate * scale * scale_font * translate1;

    auto c = glm::vec4(0.0, 0.0, 0.0, 0.0);

    auto p = m_GlyphMatrixColors.insert(std::pair<GlyphPtr, matrix_color_vector>(glyph, matrix_color_vector{}));

    for(size_t i = 0; i < sizeof(JITTER_PATTERN) / sizeof(glm::vec2); i++) {
        glm::mat4 translate3 = glm::translate(glm::mat4(1.0), glm::vec3(JITTER_PATTERN[i].x * 72 / viewport.dpi / pt_width ,
                                                                        JITTER_PATTERN[i].y * 72 / viewport.dpi_height / pt_height,
                                                                        0));
        glm::mat4 transform_x = translate3 * transform;

        if (i % 2 == 0) {
            c = glm::vec4(i == 0 ? 1.0 : 0.0,
                          i == 2 ? 1.0 : 0.0,
                          i == 4 ? 1.0 : 0.0,
                          0.0);
        }

        p.first->second.push_back(
            {
                transform_x,
                c
            });
    }

    m_VertexCount += glyph->GetSize() / sizeof(GLfloat) / 4;

    pen.x += adv_x;

    //TOOD: kerning
    return true;
}

void TextBufferImpl::Init() {
    m_TextureGenerated = false;
    m_VertexCount = 0;

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
    m_GlyphMatrixColors.clear();

    m_TextureGenerated = false;
    m_VertexCount = 0;
}

void TextBufferImpl::GenTexture() {
    if (m_TextureGenerated) return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    GLuint buffers[2] = {0};
    glGenBuffers(sizeof(buffers) / sizeof(GLuint), buffers);

    glUseProgram(*m_ProgramId);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

    for(auto p : m_GlyphMatrixColors) {
        auto & glyph = p.first;
        auto & matrix_colors = p.second;

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, glyph->GetSize(),
                     glyph->GetAddr(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ARRAY_BUFFER, matrix_colors.size() * sizeof(matrix_color_s),
                     &matrix_colors[0], GL_STATIC_DRAW);

        //a mat4 take 4 attribute of vec4
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(matrix_color_s), reinterpret_cast<void *>(0));
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(matrix_color_s), reinterpret_cast<void *>(sizeof(glm::vec4)));
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(matrix_color_s), reinterpret_cast<void *>(sizeof(glm::vec4) * 2));
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(matrix_color_s), reinterpret_cast<void *>(sizeof(glm::vec4) * 3));
        //color
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(matrix_color_s), reinterpret_cast<void *>(sizeof(glm::vec4) * 4));

        glDrawArraysInstanced(GL_TRIANGLES, 0,
                              glyph->GetSize() / sizeof(GLfloat) / 4,
                              matrix_colors.size());
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);

    glUseProgram(0);
    glDeleteBuffers(sizeof(buffers) / sizeof(GLuint), buffers);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_TextureGenerated = true;
}

} //namespace impl

TextBufferPtr CreateTextBuffer(const viewport::viewport_s & viewport) {
    return std::make_shared<impl::TextBufferImpl>(viewport);
}

} //namespace text
} //namespace ftdgl
