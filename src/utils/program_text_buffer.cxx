#include "program.h"

namespace ftdgl {
namespace impl {
static
ProgramPtr g_TextBufferProgram = {};

static
const char * vert_source = "\n"
        "attribute vec4 position4;\n"
        "attribute vec4 color;\n"
        "attribute mat4 matrix4;\n"
        "varying vec2 _coord2;\n"
        "varying vec4 _color;\n"
        "void main() {\n"
        "	_coord2 = position4.zw;\n"
        "   _color = color;\n"
        "	gl_Position = matrix4 * vec4(position4.xy, 0.0, 1.0);\n"
        "}\n";

static
const char * frag_source = "\n"
        "varying vec4 _color;\n"
        "varying vec2 _coord2;\n"
        "void main() {\n"
        "	if (_coord2.x * _coord2.x - _coord2.y > 0.0) {\n"
        "		discard;\n"
        "	}\n"
        "\n"
        "	// Upper 4 bits: front faces\n"
        "	// Lower 4 bits: back faces\n"
        "	gl_FragColor = _color * (gl_FrontFacing ? 16.0 / 255.0 : 1.0 / 255.0);\n"
        "}\n";

} //namespace impl

ProgramPtr CreateTextBufferProgram() {
    if (!impl::g_TextBufferProgram) {
        attrib_map_s map[] = {
            {1, "color"},
            {2, "matrix4"},
            {0, "position4"},
        };
        impl::g_TextBufferProgram = CreateProgram(impl::vert_source, impl::frag_source, sizeof(map) / sizeof(attrib_map_s), map);
    }

    return impl::g_TextBufferProgram;
}

} //namespace ftdgl
