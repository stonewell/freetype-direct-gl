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
	GLuint m_Vertexbuffer;
    GLuint m_RenderTextureIndex;
    GLuint m_ColorIndex;
    GLuint m_Position2Index;
    GLuint m_RectIndex;

private:
    void Init();
    void Destroy();
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

	glGenBuffers(1, &m_Vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad),
                 screen_quad, GL_STATIC_DRAW);

    glUseProgram(*m_Program);

    m_Position2Index = glGetAttribLocation(*m_Program, "position2");

    m_RenderTextureIndex = glGetUniformLocation(*m_Program, "texture");
    m_ColorIndex = glGetUniformLocation(*m_Program, "color");
    m_RectIndex = glGetUniformLocation(*m_Program, "rect");

    glUseProgram(0);
}

void RenderImpl::Destroy() {
    glDeleteBuffers(1, &m_Vertexbuffer);
}

bool RenderImpl::RenderText(text::TextBufferPtr text_buf) {
    auto c = glm::vec4(0.0, 0.0, 0.0, 0.0);
    auto rect = glm::vec4(0.0, 0.0, 1., 1.0);

    GLuint render_texture = text_buf->GetTexture();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);

    glUseProgram(*m_Program);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vertexbuffer);

    glEnableVertexAttribArray(m_Position2Index);
    glVertexAttribPointer(m_Position2Index, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUniform4fv(m_ColorIndex, 1, &c[0]);
    glUniform4fv(m_RectIndex, 1, &rect[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_texture);
    glUniform1i(m_RenderTextureIndex, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(screen_quad) / sizeof(GLfloat) / 2);

    glBlendFunc(GL_ONE, GL_ONE);

    auto count = text_buf->GetTextAttrCount();
    auto text_attr = text_buf->GetTextAttr();

    for(uint32_t i = 0;i < count; i++, text_attr++) {
        c = glm::vec4(text_attr->color[0], text_attr->color[1], text_attr->color[2], text_attr->color[3]);
        rect = glm::vec4(text_attr->bounds[0], text_attr->bounds[1], text_attr->bounds[2], text_attr->bounds[3]);

        glUniform4fv(m_ColorIndex, 1, &c[0]);
        glUniform4fv(m_RectIndex, 1, &rect[0]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(screen_quad) / sizeof(GLfloat) / 2);
    }

    glDisableVertexAttribArray(m_Position2Index);

    glUseProgram(0);
    return true;
}
} //namespace impl

RenderPtr CreateRender() {
    return std::make_shared<impl::RenderImpl>();
}

} //namespace render
} //namespace ftdgl
