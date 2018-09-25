#include "program.h"

namespace ftdgl {
namespace impl {
static
ProgramPtr g_RenderBackgroundProgram = {};

static
const char * vert_source = "\n"
        "#version 330 core\n"
        "in vec2 position2;\n"
        "in vec4 rect;\n"
        "in vec4 color;\n"
        "out vec2 _coord2;\n"
        "out vec4 _color;\n"
        "void main() {\n"
        "	_coord2 = mix(rect.xy, rect.zw, position2 * 0.5 + 0.5);\n"
        "   _color = color;\n"
        "	gl_Position = vec4(_coord2 * 2.0 - 1.0, 0.0, 1.0);\n"
        "}\n";

static
const char * frag_source = "\n"
        "#version 330 core\n"
        "in vec2 _coord2;\n"
        "in vec4 _color;\n"
        "out vec4 output_color;\n"
        "void main() {\n"
        "	output_color = _color;\n"
        "}\n";


} //namespace impl

ProgramPtr CreateRenderBackgroundProgram() {
    if (!impl::g_RenderBackgroundProgram) {
        attrib_map_s map[] = {
            {2, "color"},
            {1, "rect"},
            {0, "position2"},
        };
        impl::g_RenderBackgroundProgram = CreateProgram(impl::vert_source, impl::frag_source, sizeof(map) / sizeof(attrib_map_s), map);
    }

    return impl::g_RenderBackgroundProgram;
}

} //namespace ftdgl
