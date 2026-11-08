#ifndef AARAMBH_GFX_PIXEL_H
#define AARAMBH_GFX_PIXEL_H

#include "aarambh/gfx/image.h"

#define PIXEL_ALPHA_MASK 0xff000000
#define PIXEL_RB_MASK    0x00ff00ff
#define PIXEL_G_MASK     0x0000ff00

static inline int clamp_int(int value, int low, int high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

static inline UINT32 pixel_mix(UINT32 destination, UINT32 source, UINT32 alpha)
{
    UINT32 inverse = 256 - alpha;
    UINT32 red_blue = (source & PIXEL_RB_MASK) * alpha + (destination & PIXEL_RB_MASK) * inverse;
    UINT32 green = (source & PIXEL_G_MASK) * alpha + (destination & PIXEL_G_MASK) * inverse;

    red_blue = (red_blue >> 8) & PIXEL_RB_MASK;
    green = (green >> 8) & PIXEL_G_MASK;

    return red_blue | green;
}

static inline UINT32 pixel_lerp(UINT32 from, UINT32 to, UINT32 amount)
{
    UINT32 inverse = 256 - amount;
    UINT32 red_blue = (from & PIXEL_RB_MASK) * inverse + (to & PIXEL_RB_MASK) * amount;
    UINT32 alpha_green = ((from >> 8) & PIXEL_RB_MASK) * inverse
                       + ((to >> 8) & PIXEL_RB_MASK) * amount;

    red_blue = (red_blue >> 8) & PIXEL_RB_MASK;
    alpha_green = alpha_green & 0xff00ff00;

    return red_blue | alpha_green;
}

static inline UINT32 pixel_scale(UINT32 pixel, UINT32 factor)
{
    UINT32 red_blue = (((pixel & PIXEL_RB_MASK) * factor) >> 8) & PIXEL_RB_MASK;
    UINT32 green = (((pixel & PIXEL_G_MASK) * factor) >> 8) & PIXEL_G_MASK;

    return red_blue | green;
}

UINT32 image_sample(const Image *image, int fixed_x, int fixed_y);

#endif
