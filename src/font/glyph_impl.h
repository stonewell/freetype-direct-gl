#pragma once

#include "glyph.h"

namespace ftdgl {
namespace impl {
GlyphPtr CreateGlyph(util::MemoryBufferPtr mem_buf, uint32_t codepoint, int unitPerEM, FT_GlyphSlot & slot);
} //namespace impl
} //namespace ftdgl
