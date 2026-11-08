#ifndef AARAMBH_THEME_KIT_H
#define AARAMBH_THEME_KIT_H

#include "aarambh/easing.h"
#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/theme.h"
#include "aarambh/timing.h"
#include "aarambh/widgets.h"

/*
 * Helpers shared by the bundled themes. Layouts are designed on a 1280x800
 * canvas; kit_x / kit_y / kit_s scale those numbers to the real screen.
 */

/* Rows of the kernel list drawn by kit_kernel_list; use it as Theme.list_rows. */
#define KIT_LIST_ROWS 5

static inline float kit_x(const Image *screen, float value)
{
    return value * screen->width / 1280.0f;
}

static inline float kit_y(const Image *screen, float value)
{
    return value * screen->height / 800.0f;
}

static inline float kit_s(const Image *screen, float value)
{
    return value * screen->height / 800.0f;
}

/* Animation state every theme needs, derived from a frame. */
typedef struct {
    float intro;
    float launch;
    int alpha;
} KitFade;

KitFade kit_fade(const ThemeFrame *frame);

/* Darkens the whole screen at the very start of the intro. */
void kit_intro_blackout(Image *screen, const ThemeFrame *frame);

/* 0..1 closeness of entry `index` to the animated selection. */
float kit_weight(const ThemeFrame *frame, int index);

/* A backdrop the size of the screen, filled with a vertical gradient. */
int kit_backdrop(Image *backdrop, const Image *screen, UINT32 top, UINT32 bottom);

/* A white round glow sprite; kit_glow tints it with any color. */
int kit_glow_sprite(Image *sprite);

/* Soft light: the glow sprite stretched over a box and tinted. */
void kit_glow(Image *destination, const Image *sprite, float center_x, float center_y,
              float width, float height, UINT32 color, int alpha);

/* Copies the cached backdrop to the screen, or clears it if there is none. */
void kit_draw_backdrop(Image *screen, const Image *backdrop);

/*
 * Index of the entry closest to the animated selection, and how visible its
 * details should be (1 when settled, 0 halfway between two entries).
 */
int kit_nearest(const ThemeFrame *frame, int entry_count, float *visibility);

/* The first face in `faces` (largest first) whose width for `text` fits. */
Face *kit_fit(Face *faces, int count, const char *text, float max_width);

/* "Default system  -  Linux 6.8.0-45-generic" plus "  -  Starting in 8". */
void kit_detail_line(char *output, int capacity, const ThemeContext *context,
                     const ThemeFrame *frame, int index, int with_status);

void kit_stars(Image *destination, int count, UINT32 seed, UINT32 color, int max_alpha,
               float max_y);

/* Entry logo tinted with `color`, or its first letter when it has none. */
void kit_logo(Image *screen, const Entry *entry, Face *letter_face, float x, float y,
              float size, UINT32 color, int alpha);

void kit_text_center(Image *screen, Face *face, float center_x, int baseline,
                     const char *text, UINT32 color, int alpha);

void kit_text_right(Image *screen, Face *face, float right, int baseline,
                    const char *text, UINT32 color, int alpha);

/* "[Enter] Start  <> Switch  v Options" or the list variant; returns its width. */
float kit_hints_width(const UiHintStyle *style, const Entry *entry, const ThemeFrame *frame,
                      int vertical);

void kit_hints(Image *screen, const UiHintStyle *style, float x, int baseline,
               const Entry *entry, const ThemeFrame *frame, int vertical, int alpha);

void kit_hints_center(Image *screen, const UiHintStyle *style, int baseline,
                      const Entry *entry, const ThemeFrame *frame, int vertical, int alpha);

/* The kernel list with a theme's colors. */
void kit_kernel_list(Image *screen, const Entry *entry, const ThemeFrame *frame,
                     Face *heading_face, Face *row_face, float x, float top, float width,
                     UINT32 text_color, UINT32 accent, int alpha);

void kit_splash(const ThemeContext *context, const ThemeFrame *frame, Face *title_face,
                Face *detail_face, const Image *backdrop, UINT32 title_color,
                UINT32 detail_color);

#endif