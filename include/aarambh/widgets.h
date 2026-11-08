#ifndef AARAMBH_WIDGETS_H
#define AARAMBH_WIDGETS_H

#include "aarambh/theme.h"

/*
 * Widgets scale with the screen height, so the same call looks right from
 * 720p to 4K. Functions that lay out a row return the x where the next item
 * should start.
 */

/* Put this in an accent field to use the selected entry's own accent color. */
#define UI_ENTRY_ACCENT 0xff000000

typedef enum {
    UI_ARROWS_HORIZONTAL,
    UI_ARROWS_VERTICAL
} UiArrowAxis;

typedef struct {
    Face *face;
    UINT32 color;
    int shadow;
} UiHintStyle;

float ui_key_cap(Image *screen, const UiHintStyle *style, float x, int baseline,
                 const char *label, int alpha);

float ui_arrows(Image *screen, const UiHintStyle *style, float x, int baseline,
                UiArrowAxis axis, int alpha);

float ui_hint(Image *screen, const UiHintStyle *style, float x, int baseline,
              const char *label, int alpha);

typedef struct {
    float x;
    float y;
    float height;
    const char *label;
    int play_icon;

    /* 0..1: how far the button is in its active (white, filled) state. */
    float active;

    /* Fill opacity when inactive, 0..256. */
    int idle_fill;

    /* Outline opacity, 0..256. */
    int border;

    /* Fill and outline color; idle text uses it too. */
    UINT32 fill_color;

    /* Text and icon color on top of the fill. */
    UINT32 text_color;
} UiButton;

float ui_button_width(const Image *screen, Face *face, const UiButton *button);
float ui_button_draw(Image *screen, Face *face, const UiButton *button, int alpha);

void ui_play_icon(Image *screen, int x, int center_y, int size, UINT32 color, int alpha);

typedef struct {
    Face *time_face;
    Face *period_face;
    float right;
    int baseline;
    float gap;
    UINT32 time_color;
    UINT32 period_color;
    int shadow;
} UiClock;

void ui_clock_draw(Image *screen, const UiClock *clock, const ThemeFrame *frame, int alpha);

typedef struct {
    Face *heading_face;
    Face *row_face;
    const char *heading;
    float x;
    float top;
    float width;
    float row_height;
    int rows;
    UINT32 heading_color;
    UINT32 text_color;
    UINT32 highlight_color;
    UINT32 accent_color;
    int shadow;
} UiOptionList;

void ui_option_list_draw(Image *screen, const UiOptionList *list, const Entry *entry,
                         const ThemeFrame *frame, int alpha);

void ui_loading_dots(Image *screen, float center_x, float center_y, float now,
                     UINT32 color, int alpha);

typedef struct {
    Face *title_face;
    Face *detail_face;

    /* Image the splash fades into; NULL uses the entry's background. */
    const Image *backdrop;

    UINT32 title_color;
    UINT32 detail_color;
    UINT32 dot_color;

    /* Extra darkening of the backdrop, 0..256. */
    int darken;
    int shadow;
} UiSplash;

void ui_launch_splash_draw(const ThemeContext *context, const ThemeFrame *frame,
                           const UiSplash *splash);

/* Dot color of the splash drawn last, so Plymouth's dots match the theme. */
UINT32 ui_launch_splash_dot_color(void);

#endif