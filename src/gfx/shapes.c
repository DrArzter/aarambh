#include "aarambh/gfx/draw.h"

#include "gfx/pixel.h"
#include "lib/math.h"

static float abs_value(float value)
{
    if (value < 0) {
        return -value;
    }
    return value;
}

static float clamp_coverage(float coverage)
{
    if (coverage > 1.0f) {
        return 1.0f;
    }
    return coverage;
}

void draw_rect(Image *destination, int x, int y, int width, int height,
               UINT32 color, int alpha)
{
    int left = clamp_int(x, 0, destination->width);
    int right = clamp_int(x + width, 0, destination->width);
    int top = clamp_int(y, 0, destination->height);
    int bottom = clamp_int(y + height, 0, destination->height);
    int row_index;
    int column;

    alpha = clamp_int(alpha, 0, 256);

    for (row_index = top; row_index < bottom; row_index++) {
        UINT32 *row = destination->pixels + row_index * destination->width;

        for (column = left; column < right; column++) {
            row[column] = pixel_mix(row[column], color, (UINT32)alpha);
        }
    }
}

void draw_frame(Image *destination, int x, int y, int width, int height,
                int thickness, UINT32 color, int alpha)
{
    int outer_width = width + thickness * 2;

    draw_rect(destination, x - thickness, y - thickness, outer_width, thickness, color, alpha);
    draw_rect(destination, x - thickness, y + height, outer_width, thickness, color, alpha);
    draw_rect(destination, x - thickness, y, thickness, height, color, alpha);
    draw_rect(destination, x + width, y, thickness, height, color, alpha);
}

void draw_ring(Image *destination, float center_x, float center_y,
               float radius, float thickness, UINT32 color, int alpha)
{
    int extent = (int)(radius + thickness) + 2;
    int x;
    int y;

    for (y = (int)center_y - extent; y <= (int)center_y + extent; y++) {
        if (y < 0 || y >= destination->height) {
            continue;
        }

        for (x = (int)center_x - extent; x <= (int)center_x + extent; x++) {
            float dx;
            float dy;
            float distance;
            float coverage;
            UINT32 *pixel;

            if (x < 0 || x >= destination->width) {
                continue;
            }

            dx = x + 0.5f - center_x;
            dy = y + 0.5f - center_y;
            distance = (float)math_sqrt(dx * dx + dy * dy) - radius;
            coverage = thickness * 0.5f + 0.5f - abs_value(distance);

            if (coverage <= 0) {
                continue;
            }

            coverage = clamp_coverage(coverage);
            pixel = destination->pixels + y * destination->width + x;
            *pixel = pixel_mix(*pixel, color, (UINT32)(coverage * alpha));
        }
    }
}

void draw_triangle(Image *destination, int x, int y, int size,
                   TriangleDirection direction, UINT32 color, int alpha)
{
    int center_y = y + size / 2;
    int i;

    for (i = 0; i < size; i++) {
        int half;

        if (direction == TRIANGLE_LEFT) {
            half = i / 2;
        } else {
            half = (size - 1 - i) / 2;
        }

        draw_rect(destination, x + i, center_y - half, 1, half * 2 + 1, color, alpha);
    }
}

static float round_rect_distance(float qx, float qy, float radius)
{
    float outside_x = 0.0f;
    float outside_y = 0.0f;
    float inside = qx;
    float distance = 0.0f;

    if (qx > 0) {
        outside_x = qx;
    }
    if (qy > 0) {
        outside_y = qy;
    }
    if (qy > inside) {
        inside = qy;
    }

    if (outside_x > 0 || outside_y > 0) {
        distance = (float)math_sqrt(outside_x * outside_x + outside_y * outside_y);
    }

    if (inside < 0) {
        distance += inside;
    }

    return distance - radius;
}

static float round_rect_coverage(float distance, float thickness)
{
    float outer;
    float inner;

    if (thickness <= 0) {
        return 0.5f - distance;
    }

    outer = 0.5f - distance;
    inner = distance + thickness + 0.5f;

    if (outer < inner) {
        return outer;
    }
    return inner;
}

void draw_round_rect(Image *destination, float x, float y, float width, float height,
                     float radius, float thickness, UINT32 color, int alpha)
{
    int left = clamp_int((int)x - 1, 0, destination->width);
    int right = clamp_int((int)(x + width) + 2, 0, destination->width);
    int top = clamp_int((int)y - 1, 0, destination->height);
    int bottom = clamp_int((int)(y + height) + 2, 0, destination->height);
    float center_x = x + width / 2;
    float center_y = y + height / 2;
    float half_x = width / 2 - radius;
    float half_y = height / 2 - radius;
    int row_index;
    int column;

    alpha = clamp_int(alpha, 0, 256);

    for (row_index = top; row_index < bottom; row_index++) {
        UINT32 *row = destination->pixels + row_index * destination->width;
        float qy = abs_value(row_index + 0.5f - center_y) - half_y;

        for (column = left; column < right; column++) {
            float qx = abs_value(column + 0.5f - center_x) - half_x;
            float distance = round_rect_distance(qx, qy, radius);
            float coverage = round_rect_coverage(distance, thickness);

            if (coverage <= 0) {
                continue;
            }

            coverage = clamp_coverage(coverage);
            row[column] = pixel_mix(row[column], color, (UINT32)(coverage * alpha));
        }
    }
}
