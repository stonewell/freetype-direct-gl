#include "memory_buffer.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <iostream>

namespace ftdgl {
namespace util {
namespace impl {
class MemoryBufferImpl : public MemoryBuffer {
public:
    MemoryBufferImpl(size_t size)
        : m_Size {size}
        , m_Inited {false}
        , m_MappedRegion{}
        , m_Offset {0}
    {
        Initialize();
    }

    virtual ~MemoryBufferImpl() {
        Cleanup();
    }

public:
    virtual uint8_t * Begin() {
        if (m_Offset >= m_Size) {
            std::cerr << "MemoryBuffer out of space"
                      << std::endl;
            return nullptr;
        }
        return reinterpret_cast<uint8_t*>(m_MappedRegion.get_address()) + m_Offset;
    }

    virtual void End(size_t size) {
        m_Offset += size;
    }
public:
    void Initialize();
    void Cleanup();

private:
    size_t m_Size;

    bool m_Inited;
    boost::interprocess::mapped_region m_MappedRegion;
    size_t m_Offset;
};

void MemoryBufferImpl::Initialize() {
    if (m_Inited)
        return;

    m_MappedRegion = boost::interprocess::anonymous_shared_memory(m_Size);
    m_Offset = 0;
}

void MemoryBufferImpl::Cleanup() {
    if (!m_Inited)
        return;
}

} //namespace impl

MemoryBufferPtr CreateMemoryBuffer(size_t size) {
    auto p = std::make_shared<impl::MemoryBufferImpl>(size);

    return p;
}

} //namespace util
} //namespace ftdgl
