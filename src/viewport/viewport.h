#pragma once

#include "font.h"

namespace ftdgl {
namespace viewport {
struct viewport_s {
    int width;
    int height;
    float dpi;
    float dpi_height;
    int line_height;//0 means using the font height
    int glyph_width;//0 means using the glyph advance x
};
} //namespace viewport
} //namespace ftdgl
