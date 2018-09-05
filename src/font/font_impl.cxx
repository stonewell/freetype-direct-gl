#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include "memory_buffer.h"
#include "font_impl.h"
#include "glyph_impl.h"
#include "err_msg.h"

#include <fontconfig/fontconfig.h>
#include <iostream>
#include <stdio.h>

namespace ftdgl {
namespace impl {

#define HRES  64
#define HRESf 64.f
#define DPI   72

struct font_desc_s {
    std::string file_name;
    double size;
    bool bold;
    bool underline;
    bool force_bold;

    bool operator == (const font_desc_s & v) {
        return file_name == v.file_name
                && size == v.size
                && bold == v.bold
                && force_bold == v.force_bold
                && underline == v.underline;
    }
};

class FontImpl : public Font {
public:
    FontImpl(util::MemoryBufferPtr mem_buf, FT_Library & library, const font_desc_s & font_desc)
        : m_FontFaceInitialized {false}
        , m_FontDesc {font_desc}
        , m_Library {library}
        , m_Face {}
        , m_MemoryBuffer {mem_buf}
        , m_Glyphs {}
    {
        InitFont();
    }

    virtual ~FontImpl()
    {
        FreeFont();
    }

public:
    virtual bool IsSameFont(const std::string & desc);
    virtual GlyphPtr LoadGlyph(uint32_t codepoint);
    virtual bool LoadGlyphs(std::vector<uint32_t> codepoints,
                            Glyphs & glyphs);
    virtual int GetPtSize() const {
        return m_FontDesc.size;
    }

    virtual float GetDescender() const {
        return m_Descender;
    }

    virtual float GetAscender() const {
        return m_Ascender;
    }

    virtual float GetHeight() const {
        return m_Height;
    }

private:
    void InitFont();
    void FreeFont();

    float m_Descender;
    float m_Ascender;
    float m_Height;
    bool m_FontFaceInitialized;
    font_desc_s m_FontDesc;

    FT_Library & m_Library;
    FT_Face m_Face;
    util::MemoryBufferPtr m_MemoryBuffer;

    Glyphs m_Glyphs;
};


static
bool
match_description(const std::string & description, font_desc_s & fd )
{

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
    std::cerr << "match_description not implemented for windows."
              << std::endl;
    return false;
#endif

    FcInit();
    FcPattern *pattern = FcNameParse((const FcChar8 *)description.c_str());
    if (!pattern)
    {
        std::cerr << "fontconfig error: could not match description "
                  <<  description
                  << std::endl;
        return false;
    }

    FcConfigSubstitute( 0, pattern, FcMatchPattern );
    FcDefaultSubstitute( pattern );
    FcResult result;
    FcPattern *match = FcFontMatch( 0, pattern, &result );
    FcPatternDestroy( pattern );

    if ( !match )
    {
        std::cerr << "fontconfig error: could not match description "
                  << description
                  << std::endl;
        return false;
    }

    FcValue value;

#define GET_VALUE(p)                                                    \
    {                                                                   \
        result = FcPatternGet( match, p, 0, &value );                   \
        if ( result )                                                   \
        {                                                               \
            std::cerr << "fontconfig error: could not get pattern value:" \
                      << #p                                             \
                      << ", font desc:"                                 \
                      << description                                    \
                      << std::endl;                                     \
            break;                                                      \
        }                                                               \
    }

    do {
        GET_VALUE(FC_FILE); fd.file_name.assign((char *)(value.u.s));
        GET_VALUE(FC_SIZE); fd.size = value.u.d;
        GET_VALUE(FC_WEIGHT); fd.bold = value.u.i > FC_WEIGHT_MEDIUM;
        GET_VALUE(FC_STYLE);
        std::string style{(char*)value.u.s};

        fd.force_bold = fd.bold && style.find("Bold") == std::string::npos;

    } while(false);

    FcPatternDestroy( match );
    return !result;
#undef GET_VALUE
}

FontPtr CreateFontFromDesc(util::MemoryBufferPtr memory_buffer,
                           FT_Library & library,
                           const std::string & desc) {
    font_desc_s fd {};

    if (!match_description(desc, fd))
        return FontPtr {};

    std::cout << "f:" << fd.file_name << ","
              << "s:" << fd.size << ","
              << "b:" << fd.bold << ","
              << "fb:" << fd.force_bold << ","
              << "u:" << fd.underline
              << std::endl;

    return std::make_shared<FontImpl>(memory_buffer, library, fd);
}

bool FontImpl::IsSameFont(const std::string & desc) {
    font_desc_s fd {};

    if (!match_description(desc, fd))
        return false;

    return m_FontDesc == fd;
}

void FontImpl::InitFont() {
    if (m_FontFaceInitialized)
        return;

    FT_Error error;

    /* Load face */
    error = FT_New_Face(m_Library, m_FontDesc.file_name.c_str(), 0, &m_Face);

    if(error) {
        err_msg(error, __LINE__);
        goto cleanup;
    }

    /* Select charmap */
    error = FT_Select_Charmap(m_Face, FT_ENCODING_UNICODE);
    if(error) {
        err_msg(error, __LINE__);
        goto cleanup_face;
    }

    /* Set char size */
    error = FT_Set_Char_Size(m_Face, (int)(m_FontDesc.size * HRES), 0, DPI * HRES, DPI);

    if(error) {
        err_msg(error, __LINE__);
        goto cleanup_face;
    }

    std::cout << "as:" << m_Face->size->metrics.ascender /64.0 << ", ds:" << m_Face->size->metrics.descender / 64.0<< std::endl;

    m_Descender = FT_MulFix(m_Face->descender, m_Face->size->metrics.y_scale) / (float)64.0;
    m_Ascender = FT_MulFix(m_Face->ascender, m_Face->size->metrics.y_scale) / (float)64.0;
    m_Height = FT_MulFix(m_Face->height, m_Face->size->metrics.y_scale) / (float)64.0;

    std::cout << "d:" << m_Descender << ", a:" << m_Ascender << ", h:" << m_Height << std::endl;
    m_FontFaceInitialized = true;
    return;

cleanup_face:
    FT_Done_Face( m_Face );
cleanup:
    return;
}

void FontImpl::FreeFont() {
    if (!m_FontFaceInitialized)
        return;
    FT_Done_Face( m_Face );
}

GlyphPtr FontImpl::LoadGlyph(uint32_t codepoint) {
    auto it = m_Glyphs.find(codepoint);

    if (it != m_Glyphs.end())
        return it->second;

    FT_UInt index = FT_Get_Char_Index(m_Face, (FT_Long)codepoint);

    FT_Error error = FT_Load_Glyph(m_Face,
                                   index,
                                   FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);
    if(error) {
        err_msg(error, __LINE__);
        return GlyphPtr {};
    }

    auto g = CreateGlyph(m_MemoryBuffer, codepoint, m_Face->units_per_EM, m_Face->glyph);

    if (g)
        m_Glyphs.emplace(codepoint, g);

    return g;
}

bool FontImpl::LoadGlyphs(std::vector<uint32_t> codepoints,
                          Glyphs & glyphs) {
    bool all_loaded = true;

    for (const auto & codepoint : codepoints) {
        auto glyph = LoadGlyph(codepoint);

        if (glyph)
            glyphs.emplace(codepoint, glyph);
        else
            all_loaded = false;
    }

    return all_loaded;
}

} //namespace impl
} //namespace ftdgl
