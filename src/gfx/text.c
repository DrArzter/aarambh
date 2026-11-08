#include "aarambh/gfx/font.h"

#include "gfx/glyph.h"
#include "gfx/pixel.h"

#define SHADOW_ALPHA 90

static void draw_glyph(Image *destination, const Glyph *glyph, int x, int y,
                       UINT32 color, int alpha)
{
    int row_index;
    int column;

    for (row_index = 0; row_index < glyph->height; row_index++) {
        int pixel_y = y + row_index;
        const unsigned char *source = glyph->bitmap + row_index * glyph->width;
        UINT32 *row;

        if (pixel_y < 0 || pixel_y >= destination->height) {
            continue;
        }

        row = destination->pixels + pixel_y * destination->width;
        for (column = 0; column < glyph->width; column++) {
            int pixel_x = x + column;

            if (pixel_x < 0 || pixel_x >= destination->width || source[column] == 0) {
                continue;
            }

            row[pixel_x] = pixel_mix(row[pixel_x], color, glyph_coverage(source[column], alpha));
        }
    }
}

void draw_text(Image *destination, Face *face, float x, int baseline,
               const char *text, UINT32 color, int alpha)
{
    float pen = x;

    if (!face_is_ready(face) || alpha <= 0) {
        return;
    }

    alpha = clamp_int(alpha, 0, 256);

    while (*text != '\0') {
        Glyph *glyph = face_glyph(face, *text);
        int glyph_x = (int)(pen + 0.5f) + glyph->x_offset;
        int glyph_y = baseline + glyph->y_offset;

        draw_glyph(destination, glyph, glyph_x, glyph_y, color, alpha);

        pen += glyph->advance;
        text++;
    }
}

void draw_text_shadow(Image *destination, Face *face, float x, int baseline,
                      const char *text, UINT32 color, int alpha)
{
    int offset = face->ascent / 24 + 1;
    int shadow_alpha = alpha * SHADOW_ALPHA / 256;

    draw_text(destination, face, x + offset, baseline + offset, text, 0, shadow_alpha);
    draw_text(destination, face, x, baseline, text, color, alpha);
}

static void draw_glyph_rotated(Image *destination, const Glyph *glyph, int x, int bottom,
                               int glyph_x, int glyph_y, UINT32 color, int alpha)
{
    int row_index;
    int column;

    for (row_index = 0; row_index < glyph->height; row_index++) {
        for (column = 0; column < glyph->width; column++) {
            unsigned char value = glyph->bitmap[row_index * glyph->width + column];
            int pixel_x;
            int pixel_y;
            UINT32 *pixel;

            if (value == 0) {
                continue;
            }

            pixel_x = x + glyph_y + row_index;
            pixel_y = bottom - (glyph_x + column);

            if (pixel_x < 0 || pixel_x >= destination->width) {
                continue;
            }
            if (pixel_y < 0 || pixel_y >= destination->height) {
                continue;
            }

            pixel = destination->pixels + pixel_y * destination->width + pixel_x;
            *pixel = pixel_mix(*pixel, color, glyph_coverage(value, alpha));
        }
    }
}

void draw_text_vertical(Image *destination, Face *face, int x, int bottom,
                        const char *text, UINT32 color, int alpha)
{
    float pen = 0;

    if (!face_is_ready(face) || alpha <= 0) {
        return;
    }

    alpha = clamp_int(alpha, 0, 256);

    while (*text != '\0') {
        Glyph *glyph = face_glyph(face, *text);
        int glyph_x = (int)(pen + 0.5f) + glyph->x_offset;
        int glyph_y = face->ascent + glyph->y_offset;

        draw_glyph_rotated(destination, glyph, x, bottom, glyph_x, glyph_y, color, alpha);

        pen += glyph->advance;
        text++;
    }
}
