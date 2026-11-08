#include "aarambh/gfx/draw.h"

#include "gfx/pixel.h"
#include "lib/math.h"

static float absolute(float value)
{
    if (value < 0) {
        return -value;
    }
    return value;
}

static float corner_coverage(float px, float py, float x, float y, float width, float height,
                             float radius)
{
    float qx = absolute(px - (x + width / 2)) - (width / 2 - radius);
    float qy = absolute(py - (y + height / 2)) - (height / 2 - radius);
    float distance;

    if (qx <= 0 || qy <= 0) {
        return 1.0f;
    }

    distance = qx * qx + qy * qy;
    if (distance <= (radius - 0.5f) * (radius - 0.5f)) {
        return 1.0f;
    }
    if (distance >= (radius + 0.5f) * (radius + 0.5f)) {
        return 0.0f;
    }

    return radius + 0.5f - (float)math_sqrt(distance);
}

void draw_backdrop(Image *destination, const Image *source, float x, float y,
                   float width, float height, float radius, int alpha)
{
    int left = clamp_int((int)x, 0, destination->width);
    int right = clamp_int((int)(x + width) + 1, 0, destination->width);
    int top = clamp_int((int)y, 0, destination->height);
    int bottom = clamp_int((int)(y + height) + 1, 0, destination->height);
    int row_index;
    int column;

    alpha = clamp_int(alpha, 0, 256);

    for (row_index = top; row_index < bottom; row_index++) {
        UINT32 *row = destination->pixels + row_index * destination->width;
        const UINT32 *from = source->pixels + row_index * source->width;

        for (column = left; column < right; column++) {
            float coverage = 1.0f;
            float px = column + 0.5f;
            float py = row_index + 0.5f;

            if (px < x || px > x + width || py < y || py > y + height) {
                continue;
            }

            if (radius > 0) {
                coverage = corner_coverage(px, py, x, y, width, height, radius);
            }

            if (coverage > 0) {
                row[column] = pixel_mix(row[column], from[column], (UINT32)(coverage * alpha));
            }
        }
    }
}
