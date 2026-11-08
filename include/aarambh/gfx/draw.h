#ifndef AARAMBH_GFX_DRAW_H
#define AARAMBH_GFX_DRAW_H

#include "aarambh/gfx/image.h"

typedef enum {
    TRIANGLE_LEFT = -1,
    TRIANGLE_RIGHT = 1
} TriangleDirection;

void draw_image(Image *destination, const Image *source,
                int x, int y, int width, int height, int alpha);

void draw_image_slice(Image *destination, const Image *source,
                      int x, int width, int source_x, int y_offset,
                      int brightness, int gradient);

void draw_rect(Image *destination, int x, int y, int width, int height,
               UINT32 color, int alpha);

void draw_frame(Image *destination, int x, int y, int width, int height,
                int thickness, UINT32 color, int alpha);

void draw_round_rect(Image *destination, float x, float y, float width, float height,
                     float radius, float thickness, UINT32 color, int alpha);

void draw_ring(Image *destination, float center_x, float center_y,
               float radius, float thickness, UINT32 color, int alpha);

void draw_triangle(Image *destination, int x, int y, int size,
                   TriangleDirection direction, UINT32 color, int alpha);

typedef struct {
    float x;
    float y;
} Point;

/* Anti-aliased line with round ends. */
void draw_line(Image *destination, float x0, float y0, float x1, float y1,
               float thickness, UINT32 color, int alpha);

/* Part of a ring from start to end, in radians, clockwise from 3 o'clock. */
void draw_arc(Image *destination, float center_x, float center_y, float radius,
              float thickness, float start, float end, UINT32 color, int alpha);

/* Filled polygon (any shape, even-odd rule), anti-aliased vertically. */
void draw_polygon(Image *destination, const Point *points, int count, UINT32 color, int alpha);

/* Only the source's alpha is used; every pixel is painted with `color`. */
void draw_image_tinted(Image *destination, const Image *source,
                       int x, int y, int width, int height, UINT32 color, int alpha);

/*
 * Copy a rounded rectangle from `source` (same size as the destination) at the
 * same position, e.g. from a blurred copy of the background for frosted glass.
 */
void draw_backdrop(Image *destination, const Image *source, float x, float y,
                   float width, float height, float radius, int alpha);

#endif
