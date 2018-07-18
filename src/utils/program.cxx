#include "program.h"
#include "shader.h"

namespace ftdgl {

ProgramPtr CreateProgram(const char * vert_source, const char * frag_source) {
    return {new GLuint[1] {shader_load(vert_source, frag_source)},
            [] (auto p) {
                glDeleteProgram(p[0]);
                delete p;
            }
    };
}

} //namespace ftdgl
