#include "program.h"
#include "shader.h"

namespace ftdgl {

ProgramPtr CreateProgram(const char * vert_source, const char * frag_source, uint32_t attrib_map_count, attrib_map_s * attrib_map) {
    return {new GLuint[1] {shader_load(vert_source, frag_source, attrib_map_count, attrib_map)},
            [] (auto p) {
                glDeleteProgram(p[0]);
                delete p;
            }
    };
}

} //namespace ftdgl
