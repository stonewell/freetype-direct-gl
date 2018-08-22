#pragma once

#include "opengl.h"

namespace ftdgl {
typedef struct __attrib_map_s {
    uint32_t pos;
    const char * name;
} attrib_map_s;

GLuint shader_load(const char * vert_source, const char * frag_source, uint32_t attrib_map_count = 0, attrib_map_s * attrib_map = nullptr);
} //namespce ftdgl
