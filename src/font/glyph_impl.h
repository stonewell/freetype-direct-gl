#pragma once

#include "glyph.h"

namespace ftdgl {
namespace impl {
GlyphPtr CreateGlyph(util::MemoryBufferPtr mem_buf, uint32_t codepoint, FT_Outline & outline);
} //namespace impl
} //namespace ftdgl
