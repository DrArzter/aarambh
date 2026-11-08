#include "aarambh/gfx/draw.h"

#include "gfx/pixel.h"

#define MAX_BLIT_COLUMNS 4096
#define MAX_SLICE_COLUMNS 8192

void draw_image(Image *destination, const Image *source,
                int x, int y, int width, int height, int alpha)
{
    static int columns[MAX_BLIT_COLUMNS];
    int left;
    int right;
    int top;
    int bottom;
    INT64 step_x;
    INT64 step_y;
    int column;
    int row_index;

    if (width <= 0 || height <= 0) {
        return;
    }

    left = clamp_int(x, 0, destination->width);
    right = clamp_int(x + width, 0, destination->width);
    top = clamp_int(y, 0, destination->height);
    bottom = clamp_int(y + height, 0, destination->height);

    if (right - left > MAX_BLIT_COLUMNS) {
        right = left + MAX_BLIT_COLUMNS;
    }

    step_x = ((INT64)source->width << 16) / width;
    step_y = ((INT64)source->height << 16) / height;

    for (column = left; column < right; column++) {
        columns[column - left] = (int)((column - x) * step_x + step_x / 2 - 32768);
    }

    for (row_index = top; row_index < bottom; row_index++) {
        int fixed_y = (int)((row_index - y) * step_y + step_y / 2 - 32768);
        UINT32 *row = destination->pixels + row_index * destination->width;

        for (column = left; column < right; column++) {
            UINT32 pixel = image_sample(source, columns[column - left], fixed_y);
            UINT32 coverage = (pixel >> 24) * (UINT32)alpha >> 8;

            coverage += coverage >> 7;
            if (coverage != 0) {
                row[column] = pixel_mix(row[column], pixel, coverage);
            }
        }
    }
}

void draw_image_slice(Image *destination, const Image *source,
                      int x, int width, int source_x, int y_offset,
                      int brightness, int gradient)
{
    static UINT32 factors[MAX_SLICE_COLUMNS];
    int left = clamp_int(x, 0, destination->width);
    int right = clamp_int(x + width, 0, destination->width);
    int max_source_x = 0;
    int column;
    int row_index;

    if (right <= left || width <= 0) {
        return;
    }

    if (source->width - width > 0) {
        max_source_x = source->width - width;
    }
    source_x = clamp_int(source_x, 0, max_source_x);

    for (column = left; column < right; column++) {
        int shade = gradient * (width - (column - x)) / width;

        factors[column - left] = (UINT32)(brightness * (256 - shade) / 256);
    }

    for (row_index = 0; row_index < destination->height; row_index++) {
        UINT32 *row = destination->pixels + row_index * destination->width;
        int source_y = row_index - y_offset;
        const UINT32 *source_row;

        if (source_y < 0 || source_y >= source->height) {
            for (column = left; column < right; column++) {
                row[column] = 0;
            }
            continue;
        }

        source_row = source->pixels + source_y * source->width + source_x + (left - x);
        for (column = left; column < right; column++) {
            row[column] = pixel_scale(source_row[column - left], factors[column - left]);
        }
    }
}

void draw_image_tinted(Image *destination, const Image *source,
                       int x, int y, int width, int height, UINT32 color, int alpha)
{
    int left = clamp_int(x, 0, destination->width);
    int right = clamp_int(x + width, 0, destination->width);
    int top = clamp_int(y, 0, destination->height);
    int bottom = clamp_int(y + height, 0, destination->height);
    INT64 step_x;
    INT64 step_y;
    int row_index;
    int column;

    if (width <= 0 || height <= 0) {
        return;
    }

    step_x = ((INT64)source->width << 16) / width;
    step_y = ((INT64)source->height << 16) / height;

    for (row_index = top; row_index < bottom; row_index++) {
        int fixed_y = (int)((row_index - y) * step_y + step_y / 2 - 32768);
        UINT32 *row = destination->pixels + row_index * destination->width;

        for (column = left; column < right; column++) {
            int fixed_x = (int)((column - x) * step_x + step_x / 2 - 32768);
            UINT32 coverage = (image_sample(source, fixed_x, fixed_y) >> 24) * (UINT32)alpha >> 8;

            coverage += coverage >> 7;
            if (coverage != 0) {
                row[column] = pixel_mix(row[column], color, coverage);
            }
        }
    }
}
