#pragma once

#include <memory>
#include "opengl.h"
#include "shader.h"

namespace ftdgl {
using ProgramPtr = std::shared_ptr<GLuint>;

ProgramPtr CreateTextBufferProgram();
ProgramPtr CreateRenderProgram();
ProgramPtr CreateRenderBackgroundProgram();
ProgramPtr CreateProgram(const char * vert_source, const char * frag_source, uint32_t attrib_map_count = 0, attrib_map_s * attrib_map = nullptr);
} //namespace ftdgl
