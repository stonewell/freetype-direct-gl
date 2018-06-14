#pragma once

#include <memory>

namespace ftdgl {

class Font {
public:
    Font() = default;
    virtual ~Font() = default;
};

using FontPtr = std::shared_ptr<Font>;
}
