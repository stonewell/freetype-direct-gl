#include "render.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "opengl.h"

#include "program.h"

namespace ftdgl {
namespace render {
namespace impl {
class RenderImpl : public Render {
public:
    RenderImpl()
        : m_Program{CreateRenderProgram()} {
    }

    virtual ~RenderImpl() {}

    virtual bool RenderText(text::TextBufferPtr text_buf);

private:
    ProgramPtr m_Program;
};

static
const GLfloat screen_quad[] = {
    -1, -1, 1, -1, -1, 1, 1, 1
};

bool RenderImpl::RenderText(text::TextBufferPtr text_buf) {
    GLuint render_texture = text_buf->GetTexture();

    auto c = glm::vec4(0.0, 0.0, 0.0, 0.0);
    auto rect = glm::vec4(0.0, 0.0, 1.0, 1.0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad),
                 screen_quad, GL_STATIC_DRAW);

    glUseProgram(*m_Program);

    GLint posAttrib = glGetAttribLocation(*m_Program, "position2");
    glEnableVertexAttribArray(posAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_texture);
    glUniform1i(glGetUniformLocation(*m_Program, "texture"), 0);

    glUniform4fv(glGetUniformLocation(*m_Program, "color"),
                 1, &c[0]);
    glUniform4fv(glGetUniformLocation(*m_Program, "rect"),
                 1, &rect[0]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(screen_quad) / sizeof(GLfloat) / 2);

    glBlendFunc(GL_ONE, GL_ONE);
    c = glm::vec4(.0, .0, 1.0, 1.0);
    glUniform4fv(glGetUniformLocation(*m_Program, "color"),
                 1, &c[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(screen_quad) / sizeof(GLfloat) / 2);

    glDisableVertexAttribArray(posAttrib);
    glUseProgram(0);
    return true;
}
} //namespace impl

RenderPtr CreateRender() {
    return std::make_shared<impl::RenderImpl>();
}

} //namespace render
} //namespace ftdgl
