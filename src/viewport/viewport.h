#pragma once

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
};
} //namespace viewport
} //namespace ftdgl
