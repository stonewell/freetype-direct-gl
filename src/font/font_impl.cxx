#include "font_impl.h"

#include <fontconfig/fontconfig.h>
#include <iostream>

namespace ftdgl {
namespace impl {

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
    FontImpl(const font_desc_s & font_desc)
        : m_FontDesc {font_desc} {
    }
    virtual ~FontImpl() = default;

public:
    virtual bool IsSameFont(const std::string & desc);

private:
    font_desc_s m_FontDesc;
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

FontPtr CreateFontFromDesc(const std::string & desc) {
    font_desc_s fd {};

    if (!match_description(desc, fd))
        return FontPtr {};

    std::cout << "f:" << fd.file_name << ","
              << "s:" << fd.size << ","
              << "b:" << fd.bold << ","
              << "fb:" << fd.force_bold << ","
              << "u:" << fd.underline
              << std::endl;

    return std::make_shared<FontImpl>(fd);
}

bool FontImpl::IsSameFont(const std::string & desc) {
    font_desc_s fd {};

    if (!match_description(desc, fd))
        return false;

    return m_FontDesc == fd;
}

}
}
