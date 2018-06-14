#pragma once

#include <memory>

namespace ftdgl {
namespace util {
class MemoryBuffer {
public:
    MemoryBuffer() = default;
    virtual ~MemoryBuffer() = default;
    virtual uint8_t * Begin() = 0;
    virtual void End(size_t size) = 0;
};

using MemoryBufferPtr = std::shared_ptr<MemoryBuffer>;

MemoryBufferPtr CreateMemoryBuffer(size_t size);
} //namespace util
} //namespace ftdgl
