#include "font_manager.h"
#include "font_impl.h"

namespace ftdgl {
namespace impl {

class FontManagerImpl : public FontManager {
public:
    FontManagerImpl() = default;
    virtual ~FontManagerImpl() = default;

public:
    virtual FontPtr CreateFontFromDesc(const std::string &desc);
};

FontPtr FontManagerImpl::CreateFontFromDesc(const std::string &desc) {
    return impl::CreateFontFromDesc(desc);
}
} // namespace impl

FontManagerPtr CreateFontManager() {
    return std::make_shared<impl::FontManagerImpl>();
}
} // namespace ftdgl
