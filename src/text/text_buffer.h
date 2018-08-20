#pragma once

#include <string>
#include <memory>

#include "pen.h"
#include "markup.h"
#include "viewport.h"

namespace ftdgl {
namespace text {

typedef struct  __text_attr_s {
    double bounds[4];
    double color[4];
    double back_color[4];
} text_attr_s;

class TextBuffer {
public:
    TextBuffer() = default;
    virtual ~TextBuffer() = default;

public:
    virtual bool AddText(pen_s & pen, const markup_s & markup, const std::wstring & text) = 0;
    virtual void Clear() = 0;
    virtual uint32_t GetTexture() const = 0;
    virtual uint32_t GetTextAttrCount() const = 0;
    virtual const text_attr_s * GetTextAttr() const = 0;
};

using TextBufferPtr = std::shared_ptr<TextBuffer>;

TextBufferPtr CreateTextBuffer(const viewport::viewport_s & viewport);

} //namespace text
} //namespace ftdgl
