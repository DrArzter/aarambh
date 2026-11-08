#include "aarambh/gfx/draw.h"

#include "gfx/pixel.h"
#include "lib/math.h"
#include "lib/memory.h"

#define POLYGON_SUBSAMPLES 4
#define MAX_POLYGON_CROSSINGS 64

static float minf(float a, float b)
{
    if (a < b) {
        return a;
    }
    return b;
}

static float maxf(float a, float b)
{
    if (a > b) {
        return a;
    }
    return b;
}

static float absolute(float value)
{
    if (value < 0) {
        return -value;
    }
    return value;
}

static float segment_distance(float px, float py, float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float length = dx * dx + dy * dy;
    float t = 0.0f;
    float cx;
    float cy;

    if (length > 0.0f) {
        t = ((px - x0) * dx + (py - y0) * dy) / length;
        t = maxf(0.0f, minf(1.0f, t));
    }

    cx = x0 + t * dx - px;
    cy = y0 + t * dy - py;
    return (float)math_sqrt(cx * cx + cy * cy);
}

static void blend_pixel(Image *destination, int x, int y, UINT32 color, float coverage, int alpha)
{
    UINT32 *pixel;

    if (coverage <= 0.0f) {
        return;
    }
    if (coverage > 1.0f) {
        coverage = 1.0f;
    }

    pixel = destination->pixels + y * destination->width + x;
    *pixel = pixel_mix(*pixel, color, (UINT32)(coverage * alpha));
}

void draw_line(Image *destination, float x0, float y0, float x1, float y1,
               float thickness, UINT32 color, int alpha)
{
    float half = thickness / 2.0f;
    int left = clamp_int((int)(minf(x0, x1) - half) - 1, 0, destination->width);
    int right = clamp_int((int)(maxf(x0, x1) + half) + 2, 0, destination->width);
    int top = clamp_int((int)(minf(y0, y1) - half) - 1, 0, destination->height);
    int bottom = clamp_int((int)(maxf(y0, y1) + half) + 2, 0, destination->height);
    int x;
    int y;

    alpha = clamp_int(alpha, 0, 256);

    for (y = top; y < bottom; y++) {
        for (x = left; x < right; x++) {
            float distance = segment_distance(x + 0.5f, y + 0.5f, x0, y0, x1, y1);

            blend_pixel(destination, x, y, color, half + 0.5f - distance, alpha);
        }
    }
}

static int angle_in_range(float angle, float start, float end)
{
    float turn = (float)(2.0 * MATH_PI);

    while (angle < start) {
        angle += turn;
    }
    while (angle > start + turn) {
        angle -= turn;
    }

    return angle <= end;
}

void draw_arc(Image *destination, float center_x, float center_y, float radius,
              float thickness, float start, float end, UINT32 color, int alpha)
{
    int extent = (int)(radius + thickness) + 2;
    int left = clamp_int((int)center_x - extent, 0, destination->width);
    int right = clamp_int((int)center_x + extent, 0, destination->width);
    int top = clamp_int((int)center_y - extent, 0, destination->height);
    int bottom = clamp_int((int)center_y + extent, 0, destination->height);
    int x;
    int y;

    alpha = clamp_int(alpha, 0, 256);

    for (y = top; y < bottom; y++) {
        for (x = left; x < right; x++) {
            float dx = x + 0.5f - center_x;
            float dy = y + 0.5f - center_y;
            float distance = (float)math_sqrt(dx * dx + dy * dy) - radius;
            float coverage = thickness * 0.5f + 0.5f - absolute(distance);

            if (coverage <= 0.0f) {
                continue;
            }

            if (angle_in_range((float)math_atan2(dy, dx), start, end)) {
                blend_pixel(destination, x, y, color, coverage, alpha);
            }
        }
    }
}

static int polygon_crossings(const Point *points, int count, float y, float *crossings)
{
    int found = 0;
    int i;

    for (i = 0; i < count && found < MAX_POLYGON_CROSSINGS; i++) {
        const Point *a = &points[i];
        const Point *b = &points[(i + 1) % count];

        if ((a->y <= y && b->y > y) || (b->y <= y && a->y > y)) {
            crossings[found] = a->x + (y - a->y) / (b->y - a->y) * (b->x - a->x);
            found++;
        }
    }

    return found;
}

static void sort_floats(float *values, int count)
{
    int i;
    int j;

    for (i = 1; i < count; i++) {
        float value = values[i];

        for (j = i - 1; j >= 0 && values[j] > value; j--) {
            values[j + 1] = values[j];
        }
        values[j + 1] = value;
    }
}

static void accumulate_row(float *coverage, int width, const float *crossings, int count)
{
    int i;

    for (i = 0; i + 1 < count; i += 2) {
        float start = maxf(crossings[i], 0.0f);
        float end = minf(crossings[i + 1], (float)width);
        int first = (int)start;
        int last = (int)end;
        int x;

        if (end <= start) {
            continue;
        }

        if (first == last) {
            coverage[first] += (end - start) / POLYGON_SUBSAMPLES;
            continue;
        }

        coverage[first] += (first + 1 - start) / POLYGON_SUBSAMPLES;
        for (x = first + 1; x < last; x++) {
            coverage[x] += 1.0f / POLYGON_SUBSAMPLES;
        }
        if (last < width) {
            coverage[last] += (end - last) / POLYGON_SUBSAMPLES;
        }
    }
}

void draw_polygon(Image *destination, const Point *points, int count, UINT32 color, int alpha)
{
    float crossings[MAX_POLYGON_CROSSINGS];
    float *coverage;
    float top = points[0].y;
    float bottom = points[0].y;
    float left = points[0].x;
    float right = points[0].x;
    int first_column;
    int last_column;
    int row;
    int i;

    if (count < 3) {
        return;
    }

    for (i = 1; i < count; i++) {
        top = minf(top, points[i].y);
        bottom = maxf(bottom, points[i].y);
        left = minf(left, points[i].x);
        right = maxf(right, points[i].x);
    }

    first_column = clamp_int((int)left, 0, destination->width);
    last_column = clamp_int((int)right + 2, 0, destination->width);

    coverage = mem_alloc(sizeof(float) * (mem_size_t)(destination->width + 1));
    if (coverage == NULL) {
        return;
    }

    alpha = clamp_int(alpha, 0, 256);

    for (row = clamp_int((int)top, 0, destination->height);
         row < clamp_int((int)bottom + 1, 0, destination->height); row++) {
        int sample;
        int x;

        mem_set(coverage, 0, sizeof(float) * (mem_size_t)(destination->width + 1));

        for (sample = 0; sample < POLYGON_SUBSAMPLES; sample++) {
            float y = row + (sample + 0.5f) / POLYGON_SUBSAMPLES;
            int found = polygon_crossings(points, count, y, crossings);

            sort_floats(crossings, found);
            accumulate_row(coverage, destination->width, crossings, found);
        }

        for (x = first_column; x < last_column; x++) {
            blend_pixel(destination, x, row, color, coverage[x], alpha);
        }
    }

    mem_free(coverage);
}
