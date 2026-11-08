#ifndef AARAMBH_GFX_GLYPH_H
#define AARAMBH_GFX_GLYPH_H

#include "aarambh/gfx/font.h"

Glyph *face_glyph(Face *face, char character);

static inline UINT32 glyph_coverage(unsigned char value, int alpha)
{
    return (UINT32)((value + (value >> 7)) * alpha >> 8);
}

#endif
