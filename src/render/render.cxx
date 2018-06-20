#include "render.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "opengl.h"
#include "shader.h"

namespace ftdgl {
namespace render {
namespace impl {
class RenderImpl : public Render {
public:
    virtual bool RenderText(text::TextBufferPtr text_buf);
};

static
const char * vert_source = "\n"
        "attribute vec2 position2;\n"
        "varying vec2 _coord2;\n"
        "uniform vec4 rect;\n"
        "void main() {\n"
        "	_coord2 = mix(rect.xy, rect.zw, position2 * 0.5 + 0.5);\n"
        "	gl_Position = vec4(_coord2 * 2.0 - 1.0, 0.0, 1.0);\n"
        "}\n";

static
const char * frag_source = "\n"
        "uniform sampler2D texture;\n"
        "varying vec2 _coord2;\n"
        "uniform vec4 color;\n"
        "void main() {\n"
        "	// Get samples for -2/3 and -1/3\n"
        "	vec2 valueL = texture2D(texture, vec2(_coord2.x + dFdx(_coord2.x), _coord2.y)).yz * 255.0;\n"
        "	vec2 lowerL = mod(valueL, 16.0);\n"
        "	vec2 upperL = (valueL - lowerL) / 16.0;\n"
        "	vec2 alphaL = min(abs(upperL - lowerL), 2.0);\n"
        "\n"
        "	// Get samples for 0, +1/3, and +2/3\n"
        "	vec3 valueR = texture2D(texture, _coord2).xyz * 255.0;\n"
        "	vec3 lowerR = mod(valueR, 16.0);\n"
        "	vec3 upperR = (valueR - lowerR) / 16.0;\n"
        "	vec3 alphaR = min(abs(upperR - lowerR), 2.0);\n"
        "\n"
        "	// Average the energy over the pixels on either side\n"
        "	vec4 rgba = vec4(\n"
        "		(alphaR.x + alphaR.y + alphaR.z) / 6.0,\n"
        "		(alphaL.y + alphaR.x + alphaR.y) / 6.0,\n"
        "		(alphaL.x + alphaL.y + alphaR.x) / 6.0,\n"
        "		0.0);\n"
        "\n"
        "	// Optionally scale by a color\n"
        "	gl_FragColor = color.a == 0.0 ? 1.0 - rgba : color * rgba;\n"
        "}\n";

// static
// const char * vert_source = "\n"
//         "attribute vec2 position2;\n"
//         "varying vec2 _coord2;\n"
//         "uniform vec4 rect;\n"
//         "void main() {\n"
//         "	_coord2 = position2;\n"
//         "	gl_Position = vec4(position2, 0.0, 1.0);\n"
//         "}\n";
// static
// const char * frag_source = "\n"
//         "uniform sampler2D texture;\n"
//         "varying vec2 _coord2;\n"
//         "uniform vec4 color;\n"
//         "void main() {\n"
//         "	gl_FragColor = texture2D(texture, _coord2);\n"
//         "}\n";
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

    GLuint program = shader_load(vert_source, frag_source);
    glUseProgram(program);

    GLint posAttrib = glGetAttribLocation(program, "position2");
    glEnableVertexAttribArray(posAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_texture);
    glUniform1i(glGetUniformLocation(program, "texture"), 0);

    glUniform4fv(glGetUniformLocation(program, "color"),
                 1, &c[0]);
    glUniform4fv(glGetUniformLocation(program, "rect"),
                 1, &rect[0]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(screen_quad) / sizeof(GLfloat) / 2);

    glBlendFunc(GL_ONE, GL_ONE);
    c = glm::vec4(.0, .0, 1.0, 1.0);
    glUniform4fv(glGetUniformLocation(program, "color"),
                 1, &c[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(screen_quad) / sizeof(GLfloat) / 2);

    glDisableVertexAttribArray(posAttrib);
    glUseProgram(0);
    return false;
}
} //namespace impl

RenderPtr CreateRender() {
    return std::make_shared<impl::RenderImpl>();
}

} //namespace render
} //namespace ftdgl
