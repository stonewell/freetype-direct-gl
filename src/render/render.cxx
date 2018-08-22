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
	GLuint m_Vertexbuffer;
    GLuint m_RenderTextureIndex;
    GLuint m_ColorIndex;
    GLuint m_Position2Index;
    GLuint m_RectIndex;
    GLuint m_FirstRoundIndex;
    GLuint m_ColorBackgroundIndex;
    GLuint m_Position2BackgroundIndex;
    GLuint m_RectBackgroundIndex;
    GLuint m_RectColorBuffer;

private:
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

void RenderImpl::Init() {
    m_Program = CreateRenderProgram();
    m_ProgramBackground = CreateRenderBackgroundProgram();

	glGenBuffers(1, &m_Vertexbuffer);
    glGenBuffers(1, &m_RectColorBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad),
                 screen_quad, GL_STATIC_DRAW);

    glUseProgram(*m_Program);

    m_Position2Index = glGetAttribLocation(*m_Program, "position2");
    m_ColorIndex = glGetAttribLocation(*m_Program, "color");
    m_RectIndex = glGetAttribLocation(*m_Program, "rect");

    m_RenderTextureIndex = glGetUniformLocation(*m_Program, "texture");
    m_FirstRoundIndex = glGetUniformLocation(*m_Program, "first_round");

    glUseProgram(0);

    glUseProgram(*m_ProgramBackground);

    m_Position2BackgroundIndex = glGetAttribLocation(*m_ProgramBackground, "position2");
    m_ColorBackgroundIndex = glGetAttribLocation(*m_ProgramBackground, "color");
    m_RectBackgroundIndex = glGetAttribLocation(*m_ProgramBackground, "rect");

    glUseProgram(0);
}

void RenderImpl::Destroy() {
    glDeleteBuffers(1, &m_Vertexbuffer);
    glDeleteBuffers(1, &m_RectColorBuffer);
}

void RenderImpl::DrawBackground(text::TextBufferPtr text_buf) {
	auto count = text_buf->GetTextAttrCount();

	//draw background
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram (*m_ProgramBackground);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vertexbuffer);
    glEnableVertexAttribArray (m_Position2BackgroundIndex);
	glVertexAttribPointer(m_Position2BackgroundIndex, 2, GL_FLOAT, GL_FALSE, 0,
                          0);
    glVertexAttribDivisor(m_Position2BackgroundIndex, 0);

    // color and rect
	glBindBuffer(GL_ARRAY_BUFFER, m_RectColorBuffer);

    glEnableVertexAttribArray(m_ColorBackgroundIndex);
	glVertexAttribPointer(m_ColorBackgroundIndex,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, back_color)));
    glVertexAttribDivisor(m_ColorBackgroundIndex, 1);

    glEnableVertexAttribArray(m_RectBackgroundIndex);
	glVertexAttribPointer(m_RectBackgroundIndex,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, bounds)));

    glVertexAttribDivisor(m_RectBackgroundIndex, 1);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,
                          sizeof(screen_quad) / sizeof(GLfloat) / 2,
                          count);

    glDisableVertexAttribArray(m_Position2BackgroundIndex);
    glDisableVertexAttribArray(m_ColorBackgroundIndex);
    glDisableVertexAttribArray(m_RectBackgroundIndex);

    glUseProgram(0);
}

void RenderImpl::DrawForeground(text::TextBufferPtr text_buf) {
	auto count = text_buf->GetTextAttrCount();

	//draw foreground
	GLuint render_texture = text_buf->GetTexture();

	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glUseProgram (*m_Program);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vertexbuffer);
	glEnableVertexAttribArray(m_Position2Index);
	glVertexAttribPointer(m_Position2Index, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(m_Position2Index, 0);

    // color and rect
	glBindBuffer(GL_ARRAY_BUFFER, m_RectColorBuffer);

    glEnableVertexAttribArray(m_ColorBackgroundIndex);
	glVertexAttribPointer(m_ColorBackgroundIndex,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, color)));
    glVertexAttribDivisor(m_ColorBackgroundIndex, 1);

    glEnableVertexAttribArray(m_RectBackgroundIndex);
	glVertexAttribPointer(m_RectBackgroundIndex,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(text::text_attr_s),
                          reinterpret_cast<void*>(offsetof(text::text_attr_s, bounds)));

    glVertexAttribDivisor(m_RectBackgroundIndex, 1);

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

    glDisableVertexAttribArray(m_Position2Index);
    glDisableVertexAttribArray(m_ColorIndex);
    glDisableVertexAttribArray(m_RectIndex);

    glUseProgram(0);
}

bool RenderImpl::RenderText(text::TextBufferPtr text_buf) {
	auto count = text_buf->GetTextAttrCount();
	auto text_attr = text_buf->GetTextAttr();

	glEnable (GL_BLEND);

	glBindBuffer(GL_ARRAY_BUFFER, m_RectColorBuffer);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(text::text_attr_s) * count,
                 text_attr, GL_STATIC_DRAW);

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
