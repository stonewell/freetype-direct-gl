#pragma once

#include <memory>
#include <unordered_map>

namespace ftdgl {
class Glyph {
public:
    Glyph() = default;
    virtual ~Glyph() = default;
    virtual uint32_t GetCodepoint() const = 0;
    virtual uint8_t * GetAddr() const = 0;
    virtual size_t GetSize() const = 0;
    virtual float GetAdvanceX() const = 0;
    virtual float GetAdvanceY() const = 0;
};

using GlyphPtr = std::shared_ptr<Glyph>;
using Glyphs = std::unordered_map<uint32_t, GlyphPtr>;
} //namespace ftdgl
