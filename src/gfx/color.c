#include "aarambh/gfx/color.h"

static UINT32 scale_channel(UINT32 channel, int factor)
{
    UINT32 value = channel * (UINT32)factor >> 8;

    if (value > 255) {
        return 255;
    }
    return value;
}

UINT32 color_shade(UINT32 color, int factor)
{
    UINT32 red = scale_channel((color >> 16) & 0xff, factor);
    UINT32 green = scale_channel((color >> 8) & 0xff, factor);
    UINT32 blue = scale_channel(color & 0xff, factor);

    return (color & 0xff000000) | (red << 16) | (green << 8) | blue;
}
