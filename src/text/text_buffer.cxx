#include "text_buffer.h"

namespace ftdgl {
namespace text {
namespace impl {
class TextBufferImpl : public TextBuffer {
public:
    virtual bool AddText(pen_s & pen, const markup_s & markup, const std::wstring & text);
};

bool TextBufferImpl::AddText(pen_s & pen, const markup_s & markup, const std::wstring & text) {
    (void)pen;
    (void)text;
    (void)markup;
    return false;
}

} //namespace impl

TextBufferPtr CreateTextBuffer() {
    return std::make_shared<impl::TextBufferImpl>();
}

} //namespace text
} //namespace ftdgl
