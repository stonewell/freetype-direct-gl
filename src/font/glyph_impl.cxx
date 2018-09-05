#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include "memory_buffer.h"
#include "glyph_impl.h"
#include "glyph_compiler.h"

#include <fontconfig/fontconfig.h>
#include <iostream>
#include <stdio.h>

namespace ftdgl {
namespace impl {

class GlyphImpl : public Glyph {
public:
    GlyphImpl(uint32_t codepoint, int unitPerEM, FT_GlyphSlot & slot, uint8_t * addr, size_t size)
        // : m_UnitPerEM{unitPerEM}
          : m_Codepoint{codepoint}
          , m_Addr{addr}
        , m_Size{size}
    {
        (void)unitPerEM;
        InitGlyph(slot);
    }

    virtual ~GlyphImpl() {
    }

public:
    void InitGlyph(const FT_GlyphSlot & slot) {
        m_AdvanceX = FT_MulFix(slot->advance.x, slot->face->size->metrics.y_scale) / 64.0;
        m_AdvanceY = FT_MulFix(slot->advance.y, slot->face->size->metrics.y_scale) / 64.0;
    }

public:
    virtual uint32_t GetCodepoint() const { return m_Codepoint; }
    virtual uint8_t * GetAddr() const { return m_Addr; }
    virtual size_t GetSize() const { return m_Size; }
    virtual float GetAdvanceX() const { return m_AdvanceX; }
    virtual float GetAdvanceY() const { return m_AdvanceY; }
    virtual bool NeedDraw() const { return m_Addr != nullptr; }

private:
    // int m_UnitPerEM;
    uint32_t m_Codepoint;
    float m_AdvanceX;
    float m_AdvanceY;
    uint8_t * m_Addr;
    size_t m_Size;
};

static
bool OutlineExist(FT_GlyphSlot & slot) {
    FT_Outline &outline = slot->outline;

    if (slot->format != FT_GLYPH_FORMAT_OUTLINE)
        return false; // Should never happen.  Just an extra check.

    if (outline.n_contours <= 0 || outline.n_points <= 0)
        return false; // Can happen for some font files.

    FT_Error error = FT_Outline_Check(&outline);

    return !error;
}

GlyphPtr CreateGlyph(util::MemoryBufferPtr mem_buf, uint32_t codepoint, int unitPerEM, FT_GlyphSlot & slot) {
    uint8_t * addr = nullptr;

    size_t size = 0;


    if (OutlineExist(slot)) {
        addr = mem_buf->Begin();
        size = compile_glyph(addr, unitPerEM, slot->outline);

        mem_buf->End(size);
    }

    return std::make_shared<GlyphImpl>(codepoint, unitPerEM, slot, addr, size);
}

} //namespace impl
} //namespace ftdgl
