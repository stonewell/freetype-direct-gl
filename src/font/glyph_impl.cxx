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
    GlyphImpl(uint32_t codepoint, FT_GlyphSlot & slot, uint8_t * addr, size_t size)
        : m_Codepoint{codepoint}
        , m_Addr{addr}
        , m_Size{size}
    {
        InitGlyph(slot);
    }

    virtual ~GlyphImpl() {
    }

public:
    void InitGlyph(const FT_GlyphSlot & slot) {
        (void)slot;
        m_AdvanceX = slot->advance.x / 64.0;
        m_AdvanceY = slot->advance.y / 64.0;
    }

public:
    virtual uint32_t GetCodepoint() const { return m_Codepoint; }
    virtual uint8_t * GetAddr() const { return m_Addr; }
    virtual size_t GetSize() const { return m_Size; }
    virtual float GetAdvanceX() const { return m_AdvanceX; }
    virtual float GetAdvanceY() const { return m_AdvanceY; }

private:
    uint32_t m_Codepoint;
    float m_AdvanceX;
    float m_AdvanceY;
    uint8_t * m_Addr;
    size_t m_Size;
};

GlyphPtr CreateGlyph(util::MemoryBufferPtr mem_buf, uint32_t codepoint, int unitPerEM, FT_GlyphSlot & slot) {
    uint8_t * addr = mem_buf->Begin();

    size_t size = compile_glyph(addr, unitPerEM, slot->outline);

    mem_buf->End(size);

    return std::make_shared<GlyphImpl>(codepoint, slot, addr, size);
}

} //namespace impl
} //namespace ftdgl
