#pragma once

#include <string>
#include <memory>

#include "pen.h"
#include "markup.h"

namespace ftdgl {
namespace text {

class TextBuffer {
public:
    TextBuffer() = default;
    virtual ~TextBuffer() = default;

public:
    virtual bool AddText(pen_s & pen, const markup_s & markup, const std::wstring & text) = 0;
    virtual uint32_t GetTexture() const = 0;
};

using TextBufferPtr = std::shared_ptr<TextBuffer>;

TextBufferPtr CreateTextBuffer();

} //namespace text
} //namespace ftdgl
