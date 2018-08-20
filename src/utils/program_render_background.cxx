#include "program.h"

namespace ftdgl {
namespace impl {
static
ProgramPtr g_RenderBackgroundProgram = {};

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
        "varying vec2 _coord2;\n"
        "uniform vec4 color;\n"
        "void main() {\n"
        "	gl_FragColor = color;\n"
        "}\n";


} //namespace impl

ProgramPtr CreateRenderBackgroundProgram() {
    if (!impl::g_RenderBackgroundProgram) {
        impl::g_RenderBackgroundProgram = CreateProgram(impl::vert_source, impl::frag_source);
    }

    return impl::g_RenderBackgroundProgram;
}

} //namespace ftdgl
