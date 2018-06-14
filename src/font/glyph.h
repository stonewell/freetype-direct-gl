#pragma once

#include <memory>
#include <unordered_map>

namespace ftdgl {
class Glyph {
public:
    Glyph() = default;
    virtual ~Glyph() = default;
};

using GlyphPtr = std::shared_ptr<Glyph>;
using Glyphs = std::unordered_map<uint32_t, GlyphPtr>;
} //namespace ftdgl
