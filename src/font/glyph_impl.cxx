#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include "memory_buffer.h"
#include "glyph_impl.h"

#include <fontconfig/fontconfig.h>
#include <iostream>
#include <stdio.h>

namespace ftdgl {
namespace impl {

class GlyphImpl : public Glyph {
};

GlyphPtr CreateGlyph(util::MemoryBufferPtr mem_buf, uint32_t codepoint, FT_Outline & outline) {
    (void)codepoint;
    (void)outline;
    (void)mem_buf;
    return std::make_shared<GlyphImpl>();
}

} //namespace impl
} //namespace ftdgl
