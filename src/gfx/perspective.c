#include "aarambh/gfx/draw.h"

#include "gfx/pixel.h"

typedef struct {
    float x;
    float center_y;
    float width;
    float left_height;
    float right_height;
} Strip;

static float overlap(float start, float end, int pixel)
{
    float low = start;
    float high = end;

    if (pixel > low) {
        low = (float)pixel;
    }
    if (pixel + 1 < high) {
        high = (float)(pixel + 1);
    }
    if (high <= low) {
        return 0.0f;
    }
    return high - low;
}

/* Perspective-correct horizontal source position: heights stand in for 1/z. */
static int source_column(const Image *source, const Strip *strip, float t)
{
    float near = (1.0f - t) * strip->left_height + t * strip->right_height;
    float u = t * strip->right_height / near;

    return (int)(u * source->width * 65536.0f) - 32768;
}

static void blend(UINT32 *pixel, UINT32 color, float coverage, int alpha)
{
    UINT32 amount = (UINT32)((color >> 24) * coverage * alpha) >> 8;

    amount += amount >> 7;
    if (amount > 256) {
        amount = 256;
    }
    if (amount != 0) {
        *pixel = pixel_mix(*pixel, color, amount);
    }
}

static void draw_strip(Image *destination, const Image *source, const Strip *strip,
                       float gap, float length, int alpha)
{
    int left = clamp_int((int)strip->x, 0, destination->width);
    int right = clamp_int((int)(strip->x + strip->width) + 1, 0, destination->width);
    int column;

    if (strip->width <= 0 || source->pixels == NULL || alpha <= 0) {
        return;
    }

    for (column = left; column < right; column++) {
        float column_cover = overlap(strip->x, strip->x + strip->width, column);
        float t = (column + 0.5f - strip->x) / strip->width;
        float height = strip->left_height + (strip->right_height - strip->left_height) * t;
        float top = strip->center_y - height / 2;
        float start = top;
        float end = top + height;
        int fixed_x;
        int first;
        int last;
        int row_index;

        if (column_cover <= 0.0f || height <= 0.0f) {
            continue;
        }

        if (length > 0.0f) {
            start = top + height + gap;
            end = start + height * length;
        }

        fixed_x = source_column(source, strip, t);
        first = clamp_int((int)start, 0, destination->height);
        last = clamp_int((int)end + 1, 0, destination->height);

        for (row_index = first; row_index < last; row_index++) {
            float cover = overlap(start, end, row_index) * column_cover;
            float v = (row_index + 0.5f - start) / height;
            float fade = 1.0f;
            UINT32 *pixel = destination->pixels + row_index * destination->width + column;

            if (cover <= 0.0f) {
                continue;
            }

            if (length > 0.0f) {
                fade = 1.0f - v / length;
                v = 1.0f - v;
            }
            if (fade <= 0.0f || v < 0.0f || v > 1.0f) {
                continue;
            }

            blend(pixel, image_sample(source, fixed_x, (int)(v * source->height * 65536.0f) - 32768),
                  cover * fade, alpha);
        }
    }
}

void draw_image_perspective(Image *destination, const Image *source, float x, float center_y,
                            float width, float left_height, float right_height, int alpha)
{
    Strip strip = { x, center_y, width, left_height, right_height };

    draw_strip(destination, source, &strip, 0.0f, 0.0f, alpha);
}

void draw_image_reflection(Image *destination, const Image *source, float x, float center_y,
                           float width, float left_height, float right_height, float gap,
                           float length, int alpha)
{
    Strip strip = { x, center_y, width, left_height, right_height };

    if (length <= 0.0f) {
        return;
    }

    draw_strip(destination, source, &strip, gap, length, alpha);
}