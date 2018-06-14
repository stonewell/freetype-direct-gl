#pragma once

#include "font.h"
#include <string>

namespace ftdgl {
namespace impl {
FontPtr CreateFontFromDesc(const std::string & desc);
} //namespace impl
} //namespace ftdgl
