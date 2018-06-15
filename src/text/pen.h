#pragma once

#include "font.h"
#include "color.h"

namespace ftdgl {
namespace text {

struct pen_s {
    double x;
    double y;

    color_s fore_color;
    color_s back_color;

    FontPtr font;
};

} //namespace text
} //namespace ftdgl
