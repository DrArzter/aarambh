#include "gfx/pixel.h"

UINT32 image_sample(const Image *image, int fixed_x, int fixed_y)
{
    int x;
    int y;
    int next_x;
    int next_y;
    UINT32 fraction_x;
    UINT32 fraction_y;
    const UINT32 *row;
    const UINT32 *next_row;
    UINT32 top;
    UINT32 bottom;

    fixed_x = clamp_int(fixed_x, 0, (image->width - 1) << 16);
    fixed_y = clamp_int(fixed_y, 0, (image->height - 1) << 16);

    x = fixed_x >> 16;
    y = fixed_y >> 16;

    next_x = x;
    if (x + 1 < image->width) {
        next_x = x + 1;
    }

    next_y = y;
    if (y + 1 < image->height) {
        next_y = y + 1;
    }

    fraction_x = (fixed_x >> 8) & 0xff;
    fraction_y = (fixed_y >> 8) & 0xff;

    row = image->pixels + y * image->width;
    next_row = image->pixels + next_y * image->width;

    top = pixel_lerp(row[x], row[next_x], fraction_x);
    bottom = pixel_lerp(next_row[x], next_row[next_x], fraction_x);

    return pixel_lerp(top, bottom, fraction_y);
}
