#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include "font_manager.h"
#include "memory_buffer.h"
#include "font_impl.h"
#include "err_msg.h"

#include <forward_list>

namespace ftdgl {
namespace impl {

using FontPtrList = std::forward_list<FontPtr>;

constexpr size_t DEFAULT_MEM_BUF_SIZE = 64 * 1024 * 1024;

class FontManagerImpl : public FontManager {
public:
    FontManagerImpl(size_t mem_buf_size)
        : m_Fonts {}
        , m_LibInited {false}
        , m_Library {}
        , m_MemoryBuffer {util::CreateMemoryBuffer(mem_buf_size)}
    {
        InitFreeTypeLib();
    }

    virtual ~FontManagerImpl() {
        //clean up loaded fonts before free library
        m_Fonts.clear();
        FreeFreeTypeLib();
    }

public:
    virtual FontPtr CreateFontFromDesc(const std::string &desc);

private:
    void InitFreeTypeLib() {
        if (m_LibInited)
            return;

        FT_Error error;

        /* Initialize library */
        error = FT_Init_FreeType(&m_Library);
        if(error) {
            err_msg(error, __LINE__);
            return;
        }

        m_LibInited = true;
    }

    void FreeFreeTypeLib() {
        FT_Done_FreeType( m_Library );
    }

    FontPtrList m_Fonts;

    bool m_LibInited;
    FT_Library m_Library;

    util::MemoryBufferPtr m_MemoryBuffer;
};

FontPtr FontManagerImpl::CreateFontFromDesc(const std::string &desc) {
    for(const auto & f : m_Fonts) {
        if (f->IsSameFont(desc)) {
            return f;
        }
    }

    auto f = impl::CreateFontFromDesc(m_MemoryBuffer, m_Library, desc);

    if (f)
        m_Fonts.push_front(f);

    return f;
}
} // namespace impl

FontManagerPtr CreateFontManager() {
    return std::make_shared<impl::FontManagerImpl>(impl::DEFAULT_MEM_BUF_SIZE);
}
} // namespace ftdgl
