#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include "err_msg.h"

#include <stdio.h>

namespace ftdgl {
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

void err_msg(FT_Error error, int line) {
    fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
            line, FT_Errors[error].code, FT_Errors[error].message);
}
} //namespace ftdgl
