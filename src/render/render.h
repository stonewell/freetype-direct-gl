#pragma once

#include "text_buffer.h"

namespace ftdgl {
namespace render {

class Render {
public:
    Render() = default;
    virtual ~Render() = default;

public:
    virtual bool RenderText(text::TextBufferPtr text_buf) = 0;
};

using RenderPtr = std::shared_ptr<Render>;

RenderPtr CreateRender();

} //namespace render
} //namespace ftdgl
