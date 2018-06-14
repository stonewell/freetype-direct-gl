#include "font_manager.h"

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
  (void)desc;
  return FontPtr{};
}
} // namespace impl

FontManagerPtr CreateFontManager() {
  return std::make_shared<impl::FontManagerImpl>();
}
} // namespace ftdgl
