#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include <cassert>
#include <iostream>

#include "opengl.h"

#include "glyph_compiler.h"
#include "err_msg.h"
#include "cu2qu.h"

namespace ftdgl {
namespace impl {

enum Kind {
    SOLID,
    QUADRATIC_CURVE
};

struct compile_context_s {
    uint8_t * addr;
    size_t size;
    int contourCount;
    FT_Pos firstX, firstY, currentX, currentY;
    int unitPerEM;
};

static int MoveToFunction(const FT_Vector *to,
                          void *user);
static int LineToFunction(const FT_Vector *to,
                          void *user);
static int ConicToFunction(const FT_Vector *control,
                           const FT_Vector *to,
                           void *user);
static int CubicToFunction(const FT_Vector *controlOne,
                           const FT_Vector *controlTwo,
                           const FT_Vector *to,
                           void *user);
static void AppendTriangle(compile_context_s * context,
                          FT_Pos x1, FT_Pos y1,
                          FT_Pos x2, FT_Pos y2,
                          FT_Pos x3, FT_Pos y3,
                          Kind kind);


size_t compile_glyph(uint8_t * addr, int unitPerEM, FT_Outline & outline) {
    FT_Outline_Funcs callbacks;

    compile_context_s context {.addr=addr, .size=0,
                .contourCount=0,
                .firstX=0, .firstY=0, .currentX=0, .currentY=0,
                .unitPerEM = unitPerEM
                };

    callbacks.move_to = MoveToFunction;
    callbacks.line_to = LineToFunction;
    callbacks.conic_to = ConicToFunction;
    callbacks.cubic_to = CubicToFunction;

    callbacks.shift = 0;
    callbacks.delta = 0;

    FT_Error error = FT_Outline_Decompose(&outline, &callbacks, &context);

    if (error) {
        err_msg(error, __LINE__);
        return 0;
    }

    return context.size;
}

int MoveToFunction(const FT_Vector *to,
                   void *user) {
    compile_context_s * context = reinterpret_cast<compile_context_s*>(user);
    context->firstX = context->currentX = to->x;
    context->firstY = context->currentY = to->y;
    context->contourCount = 0;

    return 0;
}

int LineToFunction(const FT_Vector *to,
                   void *user) {
    compile_context_s * context = reinterpret_cast<compile_context_s*>(user);
    if (++context->contourCount >= 2) {
        AppendTriangle(context, context->firstX, context->firstY, context->currentX, context->currentY,
                       to->x, to->y, SOLID);
    }

    context->currentX = to->x;
    context->currentY = to->y;
    return 0;
}

int ConicToFunction(const FT_Vector *control,
                    const FT_Vector *to,
                    void *user) {
    compile_context_s * context = reinterpret_cast<compile_context_s*>(user);
    if (++context->contourCount >= 2) {
        AppendTriangle(context, context->firstX, context->firstY, context->currentX, context->currentY,
                       to->x, to->y, SOLID);
    }

    AppendTriangle(context, context->currentX, context->currentY,
                   control->x, control->y,
                   to->x, to->y, QUADRATIC_CURVE);

    context->currentX = to->x;
    context->currentY = to->y;
    return 0;
}

int CubicToFunction(const FT_Vector *controlOne,
                    const FT_Vector *controlTwo,
                    const FT_Vector *to,
                    void *user) {
    compile_context_s * context = reinterpret_cast<compile_context_s*>(user);

    if (++context->contourCount >= 2) {
        AppendTriangle(context, context->firstX, context->firstY, context->currentX, context->currentY,
                       to->x, to->y, SOLID);
    }

    point_type_vector spline_points;
    point_type_vector ctl_points{
        {(double)context->currentX, (double)context->currentY},
        {(double)controlOne->x, (double)controlOne->y},
        {(double)controlTwo->x, (double)controlTwo->y},
        {(double)to->x, (double)to->y}
    };

    if (curve_to_quadratic(ctl_points, spline_points)) {
        auto it = spline_points.begin() + 1;

        while(it != spline_points.end() - 2) {
            auto x = std::real(*it), y = std::imag(*it);
            auto nx = std::real(*(it + 1)), ny = std::imag(*(it + 1));

            auto implied_x = .5 * (x + nx), implied_y = .5 * (y + ny);

            AppendTriangle(context, context->currentX, context->currentY,
                           x, y,
                           implied_x, implied_y,
                           QUADRATIC_CURVE);

            context->currentX = implied_x;
            context->currentY = implied_y;
            it++;
        }

        auto x = std::real(*it), y = std::imag(*it);
        auto nx = std::real(*(it + 1)), ny = std::imag(*(it + 1));

        AppendTriangle(context, context->currentX, context->currentY,
                       x, y,
                       nx, ny,
                       QUADRATIC_CURVE);
    } else {
        AppendTriangle(context, context->currentX, context->currentY,
                       controlOne->x, controlOne->y,
                       to->x, to->y, QUADRATIC_CURVE);
        AppendTriangle(context, context->currentX, context->currentY,
                       controlTwo->x, controlTwo->y,
                       to->x, to->y, QUADRATIC_CURVE);
    }

    context->currentX = to->x;
    context->currentY = to->y;
    return 0;
}

static
void AppendVertex(compile_context_s * context,
                  FT_Pos x, FT_Pos y,
                  GLfloat s, GLfloat t)
{
    GLfloat* p = reinterpret_cast<GLfloat*>(context->addr);

    *p++ = (GLfloat)x / 64.0;//context->unitPerEM;
    *p++ = (GLfloat)y / 64.0;//context->unitPerEM;
    *p++ = s;
    *p++ = t;

    context->addr += sizeof(GLfloat) * 4;
    context->size += sizeof(GLfloat) * 4;
}

void AppendTriangle(compile_context_s * context,
                   FT_Pos x1, FT_Pos y1,
                   FT_Pos x2, FT_Pos y2,
                   FT_Pos x3, FT_Pos y3,
                   Kind kind) {
    switch(kind) {
    case SOLID:
        AppendVertex(context,
                     x1, y1,
                     0, 1);
        AppendVertex(context,
                     x2, y2,
                     0, 1);
        AppendVertex(context,
                     x3, y3,
                     0, 1);
        break;
    case QUADRATIC_CURVE:
        AppendVertex(context,
                     x1, y1,
                     0, 0);
        AppendVertex(context,
                     x2, y2,
                     0.5, 0);
        AppendVertex(context,
                     x3, y3,
                     1, 1);
        break;
    }
}

} //namespace impl
} //namespace ftdgl
