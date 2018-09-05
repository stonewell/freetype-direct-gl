#pragma once

#include "font.h"

namespace ftdgl {
namespace viewport {
struct viewport_s {
    int x;
    int y;
    int pixel_width;
    int pixel_height;
    int window_width;
    int window_height;
    float dpi;
    float dpi_height;
    int line_height;//0 means using the font height
    int glyph_width;//0 means using the glyph advance x

    // double FontSizeToViewport(const FontPtr & font, double v, bool horz) const;
};
} //namespace viewport
} //namespace ftdgl
