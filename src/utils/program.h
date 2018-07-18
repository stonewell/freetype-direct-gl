#pragma once

#include <memory>
#include "opengl.h"

namespace ftdgl {
using ProgramPtr = std::shared_ptr<GLuint>;

ProgramPtr CreateTextBufferProgram();
ProgramPtr CreateRenderProgram();
ProgramPtr CreateProgram(const char * vert_source, const char * frag_source);
} //namespace ftdgl
