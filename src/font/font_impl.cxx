#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include "font_impl.h"

#include <fontconfig/fontconfig.h>
#include <iostream>
#include <stdio.h>

namespace ftdgl {
namespace impl {

#define HRES  64
#define HRESf 64.f
#define DPI   72

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
static
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

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
    FontImpl(FT_Library & library, const font_desc_s & font_desc)
        : m_FontDesc {font_desc}
        , m_FontFaceInitialized {false}
        , m_Library {library}
        , m_Face {}
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

private:
    void InitFont();
    void FreeFont();

    font_desc_s m_FontDesc;

    bool m_FontFaceInitialized;
    FT_Library & m_Library;
    FT_Face m_Face;
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

FontPtr CreateFontFromDesc(FT_Library & library, const std::string & desc) {
    font_desc_s fd {};

    if (!match_description(desc, fd))
        return FontPtr {};

    std::cout << "f:" << fd.file_name << ","
              << "s:" << fd.size << ","
              << "b:" << fd.bold << ","
              << "fb:" << fd.force_bold << ","
              << "u:" << fd.underline
              << std::endl;

    return std::make_shared<FontImpl>(library, fd);
}

bool FontImpl::IsSameFont(const std::string & desc) {
    font_desc_s fd {};

    if (!match_description(desc, fd))
        return false;

    return m_FontDesc == fd;
}

GlyphPtr FontImpl::LoadGlyph(uint32_t codepoint) {
    (void)codepoint;
    return GlyphPtr {};
}

bool FontImpl::LoadGlyphs(std::vector<uint32_t> codepoints,
                          Glyphs & glyphs) {
    (void)codepoints;
    (void)glyphs;
    return false;
}

void FontImpl::InitFont() {
    if (m_FontFaceInitialized)
        return;

    FT_Error error;
    FT_Matrix matrix = {
        (int)((1.0/HRES) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};

    /* Load face */
    error = FT_New_Face(m_Library, m_FontDesc.file_name.c_str(), 0, &m_Face);

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup;
    }

    /* Select charmap */
    error = FT_Select_Charmap(m_Face, FT_ENCODING_UNICODE);
    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_face;
    }

    /* Set char size */
    error = FT_Set_Char_Size(m_Face, (int)(m_FontDesc.size * HRES), 0, DPI * HRES, DPI);

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_face;
    }

    /* Set transform matrix */
    FT_Set_Transform(m_Face, &matrix, NULL);

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

}
}
