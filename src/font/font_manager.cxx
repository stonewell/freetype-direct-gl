#include "font_manager.h"
#include "font_impl.h"

#include <forward_list>

namespace ftdgl {
namespace impl {

using FontPtrList = std::forward_list<FontPtr>;

class FontManagerImpl : public FontManager {
public:
    FontManagerImpl() = default;
    virtual ~FontManagerImpl() = default;

public:
    virtual FontPtr CreateFontFromDesc(const std::string &desc);

private:
    FontPtrList m_Fonts;
};

FontPtr FontManagerImpl::CreateFontFromDesc(const std::string &desc) {
    for(const auto & f : m_Fonts) {
        if (f->IsSameFont(desc)) {
            return f;
        }
    }

    auto f = impl::CreateFontFromDesc(desc);

    if (f)
        m_Fonts.push_front(f);

    return f;
}
} // namespace impl

FontManagerPtr CreateFontManager() {
    return std::make_shared<impl::FontManagerImpl>();
}
} // namespace ftdgl
