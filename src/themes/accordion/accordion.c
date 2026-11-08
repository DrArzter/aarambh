#include "themes/accordion/accordion.h"

#define INTRO_STAGGER_MS     90.0f
#define PANEL_INTRO_DELAY_MS 250.0f
#define BACKGROUND_FOCUS_X   0.734f
#define BACKGROUND_PARALLAX  0.72f
#define BACKGROUND_DROP      0.35f
#define DIVIDER_ALPHA        36
#define ACCENT_BAR_HEIGHT    0.007f
#define CLOCK_MARGIN         0.065f
#define CLOCK_BASELINE       0.0926f
#define CLOCK_GAP            0.008f

AccordionFaces accordion_faces;

float accordion_launch_progress(const ThemeFrame *frame)
{
    if (!frame->launching) {
        return 0.0f;
    }
    return ease_in_out_cubic(frame->launch_elapsed / AARAMBH_LAUNCH_MS);
}

static void accordion_load(const ThemeContext *context)
{
    AccordionFaces *faces = &accordion_faces;
    float height = (float)context->screen->height;

    face_init(&faces->title, context->font_bold, height * 0.112f);
    face_init(&faces->subtitle, context->font_regular, height * 0.028f);
    face_init(&faces->button, context->font_bold, height * 0.027f);
    face_init(&faces->list, context->font_regular, height * 0.026f);
    face_init(&faces->side, context->font_bold, height * 0.03f);
    face_init(&faces->clock, context->font_bold, height * 0.056f);
    face_init(&faces->clock_period, context->font_bold, height * 0.024f);
    face_init(&faces->hint, context->font_regular, height * 0.021f);
    face_init(&faces->letter, context->font_bold, height * 0.12f);
    face_init(&faces->splash, context->font_bold, height * 0.06f);
}

static void draw_backgrounds(const ThemeContext *context, const ThemeFrame *frame,
                             const AccordionColumn *columns)
{
    Image *screen = context->screen;
    float focus_x = screen->width * BACKGROUND_FOCUS_X;
    int i;

    for (i = 0; i < context->entry_count; i++) {
        const AccordionColumn *column = &columns[i];
        float delay = i * INTRO_STAGGER_MS;
        float rise = ease_out_cubic((frame->intro_elapsed - delay) / (AARAMBH_INTRO_MS * 0.75f));
        int y_offset = (int)((1.0f - rise) * screen->height * BACKGROUND_DROP);
        int source_x = (int)(focus_x - column->width * BACKGROUND_PARALLAX);
        int brightness = (int)(140 + 116 * column->weight);
        int gradient = (int)(185 * column->weight);

        draw_image_slice(screen, &context->entries[i].background, column->x, column->width,
                         source_x, y_offset, brightness, gradient);
    }
}

static void draw_column_chrome(const ThemeContext *context, const AccordionColumn *column,
                               const Entry *entry, int is_last, float intro, float launch)
{
    Image *screen = context->screen;
    int bar_height = (int)(screen->height * ACCENT_BAR_HEIGHT) + 1;
    int bar_width = (int)(column->width * column->weight);
    int bar_alpha = (int)(256 * column->weight * intro);

    if (!is_last) {
        int divider_alpha = (int)(DIVIDER_ALPHA * intro * (1.0f - launch));

        draw_rect(screen, column->x + column->width - 1, 0, 2, screen->height,
                  COLOR_WHITE, divider_alpha);
    }

    draw_rect(screen, column->x, screen->height - bar_height, bar_width, bar_height,
              entry->accent, bar_alpha);
}

static void draw_panels(const ThemeContext *context, const ThemeFrame *frame,
                        const AccordionColumn *columns, float intro, float launch)
{
    int last = context->entry_count - 1;
    int i;

    for (i = 0; i <= last; i++) {
        const AccordionColumn *column = &columns[i];
        float delay = PANEL_INTRO_DELAY_MS + i * INTRO_STAGGER_MS;
        float appear = ease_out_cubic((frame->intro_elapsed - delay) / (AARAMBH_INTRO_MS * 0.7f));
        int base_alpha = (int)(appear * 256.0f * (1.0f - launch));
        int side_alpha = (int)(base_alpha * clampf(1.0f - column->weight * 2.0f, 0.0f, 1.0f));
        int main_alpha = (int)(base_alpha * clampf(column->weight * 2.0f - 1.0f, 0.0f, 1.0f));

        accordion_draw_side_panel(context, &context->entries[i], column, side_alpha);

        if (i == frame->selected || main_alpha > 0) {
            accordion_draw_main_panel(context, frame, i, column, main_alpha);
        }

        draw_column_chrome(context, column, &context->entries[i], i == last, intro, launch);
    }
}

static void draw_clock(const ThemeContext *context, const ThemeFrame *frame,
                       float intro, float launch)
{
    Image *screen = context->screen;
    float height = (float)screen->height;
    UiClock clock;

    clock.time_face = &accordion_faces.clock;
    clock.period_face = &accordion_faces.clock_period;
    clock.right = screen->width - height * CLOCK_MARGIN;
    clock.baseline = (int)(height * CLOCK_BASELINE);
    clock.gap = height * CLOCK_GAP;
    clock.time_color = COLOR_WHITE;
    clock.period_color = 0xebebf0;
    clock.shadow = 1;

    ui_clock_draw(screen, &clock, frame, (int)(intro * (1.0f - launch) * 256.0f));
}

static void draw_intro_fade(const ThemeContext *context, const ThemeFrame *frame)
{
    float progress = ease_out_cubic(frame->intro_elapsed / (AARAMBH_INTRO_MS * 0.55f));
    float darkness = 1.0f - progress;

    if (darkness > 0.0f) {
        image_darken(context->screen, (int)(darkness * 256.0f));
    }
}

static void accordion_render(const ThemeContext *context, const ThemeFrame *frame)
{
    AccordionColumn columns[AARAMBH_MAX_ENTRIES];
    float intro = ease_out_cubic(frame->intro_elapsed / AARAMBH_INTRO_MS);
    float launch = accordion_launch_progress(frame);

    accordion_layout(context, frame, columns);

    draw_backgrounds(context, frame, columns);
    draw_panels(context, frame, columns, intro, launch);
    draw_clock(context, frame, intro, launch);

    if (frame->launching) {
        UiSplash splash;

        splash.title_face = &accordion_faces.splash;
        splash.detail_face = &accordion_faces.subtitle;
        splash.backdrop = NULL;
        splash.title_color = COLOR_WHITE;
        splash.detail_color = 0xe8e4ee;
        splash.dot_color = COLOR_WHITE;
        splash.darken = 100;
        splash.shadow = 1;
        ui_launch_splash_draw(context, frame, &splash);
    }

    draw_intro_fade(context, frame);
}

static const EditColors accordion_edit = { 0x1d1420, 0x120c15, UI_ENTRY_ACCENT, 0xffffff, 0xb9a9bf, EDIT_FRAME_CARD };

const Theme accordion_theme = {
    .name = "accordion",
    .list_rows = ACCORDION_LIST_ROWS,
    .edit = &accordion_edit,
    .load = accordion_load,
    .render = accordion_render
};