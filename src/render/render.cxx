#include "render.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "opengl.h"

#include "program.h"

#include <iostream>

namespace ftdgl {
namespace render {
namespace impl {
class RenderImpl : public Render {
public:
    RenderImpl() {
        Init();
    }

    virtual ~RenderImpl() {
        Destroy();
    }

    virtual bool RenderText(text::TextBufferPtr text_buf);

private:
    ProgramPtr m_Program;
    ProgramPtr m_ProgramBackground;
    GLuint m_VertexArray;
	GLuint m_Vertexbuffer;
    GLuint m_RectColorBuffer;
    GLuint m_RenderTextureIndex;
    GLuint m_FirstRoundIndex;

private:
    void InitVertexArray(text::TextBufferPtr text_buf);
    void Init();
    void Destroy();

    void DrawBackground(text::TextBufferPtr text_buf);
    void DrawForeground(text::TextBufferPtr text_buf);
};

static
const GLfloat screen_quad[] = {
    -1, -1,
    1, -1,
    -1, 1,
    1, 1
};

void RenderImpl::InitVertexArray(text::TextBufferPtr text_buf) {
	auto count = text_buf->GetTextAttrCount();
	auto text_attr = text_buf->GetTextAttr();

    glBindVertexArray(m_VertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad),
                 screen_quad, GL_STATIC_DRAW);

    //position2
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(0, 0);

    // color and rect
	glBindBuffer(GL_ARRAY_BUFFER, m_RectColorBuffer);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(text::text_attr_s) * count,
                 text_attr, GL_DYNAMIC_DRAW);

    //back color
    glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, back_color)));
    glVertexAttribDivisor(2, 1);

    //fore color
    glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, color)));
    glVertexAttribDivisor(3, 1);

    //rect
    glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, bounds)));
    glVertexAttribDivisor(1, 1);
    glBindVertexArray(0);
}

void RenderImpl::Init() {
    m_Program = CreateRenderProgram();
    m_ProgramBackground = CreateRenderBackgroundProgram();

    glGenVertexArrays(1, &m_VertexArray);
	glGenBuffers(1, &m_Vertexbuffer);
    glGenBuffers(1, &m_RectColorBuffer);

    glUseProgram(*m_Program);

    m_RenderTextureIndex = glGetUniformLocation(*m_Program, "texture");
    m_FirstRoundIndex = glGetUniformLocation(*m_Program, "first_round");

    glUseProgram(0);
}

void RenderImpl::Destroy() {
    glDeleteBuffers(1, &m_Vertexbuffer);
    glDeleteBuffers(1, &m_RectColorBuffer);
    glDeleteVertexArrays(1, &m_VertexArray);
}

void RenderImpl::DrawBackground(text::TextBufferPtr text_buf) {
	auto count = text_buf->GetTextAttrCount();

	//draw background
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram (*m_ProgramBackground);

    glBindVertexArray(m_VertexArray);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,
                          sizeof(screen_quad) / sizeof(GLfloat) / 2,
                          count);

    glBindVertexArray(0);
    glUseProgram(0);
}

void RenderImpl::DrawForeground(text::TextBufferPtr text_buf) {
	auto count = text_buf->GetTextAttrCount();

	//draw foreground
	GLuint render_texture = text_buf->GetTexture();

	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glUseProgram (*m_Program);

    glBindVertexArray(m_VertexArray);

	glUniform1f(m_FirstRoundIndex, 1.0);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, render_texture);
	glUniform1i(m_RenderTextureIndex, 0);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,
                          sizeof(screen_quad) / sizeof(GLfloat) / 2,
                          count);

	glBlendFunc(GL_ONE, GL_ONE);

	glUniform1f(m_FirstRoundIndex, 0.0);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,
                          sizeof(screen_quad) / sizeof(GLfloat) / 2,
                          count);

    glUseProgram(0);
    glBindVertexArray(0);
}

bool RenderImpl::RenderText(text::TextBufferPtr text_buf) {
	glEnable (GL_BLEND);

	text_buf->GenTexture();

    InitVertexArray(text_buf);

	DrawBackground(text_buf);

	DrawForeground(text_buf);

    return true;
}

} //namespace impl

RenderPtr CreateRender() {
    return std::make_shared<impl::RenderImpl>();
}

} //namespace render
} //namespace ftdgl
