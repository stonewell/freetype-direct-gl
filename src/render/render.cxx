#include "render.h"

namespace ftdgl {
namespace render {
namespace impl {
class RenderImpl : public Render {
public:
    virtual bool RenderText(text::TextBufferPtr text_buf);
};

bool RenderImpl::RenderText(text::TextBufferPtr text_buf) {
    (void)text_buf;
    return false;
}
} //namespace impl

RenderPtr CreateRender() {
    return std::make_shared<impl::RenderImpl>();
}

} //namespace render
} //namespace ftdgl
