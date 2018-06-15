#pragma once

#include <string>
#include <memory>

#include "pen.h"

namespace ftdgl {
namespace text {

class TextBuffer {
public:
    TextBuffer() = default;
    virtual ~TextBuffer() = default;

public:
    virtual bool AddText(pen_s & pen, const std::wstring & text) = 0;
};

using TextBufferPtr = std::shared_ptr<TextBuffer>;

} //namespace text
} //namespace text
