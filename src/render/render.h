#pragma once

#include "text_buffer.h"

namespace ftdgl {
namespace render {

class Render {
public:
    Render() = default;
    virtual ~Render() = default;

public:
    virtual bool RenderText(TextBufferPtr text_buf) = 0;
};

} //namespace text
} //namespace text
