#include "aarambh/aarambh.h"
#include "aarambh/theme_kit.h"

#define MINIMAL_LIST_ROWS 5

typedef struct {
    Face title;
    Face subtitle;
    Face button;
    Face list;
    Face hint;
    Face clock;
    Face clock_period;
    Face splash;
} MinimalFaces;

static MinimalFaces faces;

static void minimal_load(const ThemeContext *context)
{
    float height = (float)context->screen->height;

    face_init(&faces.title, context->font_bold, height * 0.09f);
    face_init(&faces.subtitle, context->font_regular, height * 0.026f);
    face_init(&faces.button, context->font_bold, height * 0.026f);
    face_init(&faces.list, context->font_regular, height * 0.026f);
    face_init(&faces.hint, context->font_regular, height * 0.021f);
    face_init(&faces.clock, context->font_bold, height * 0.05f);
    face_init(&faces.clock_period, context->font_bold, height * 0.022f);
    face_init(&faces.splash, context->font_bold, height * 0.06f);
}

static void draw_background(const ThemeContext *context, const ThemeFrame *frame)
{
    int lower = (int)frame->position;
    int upper = lower + 1;
    float blend = frame->position - lower;

    image_copy(context->screen, &context->entries[lower].background);

    if (upper < context->entry_count && blend > 0.0f) {
        image_blend(context->screen, context->screen, &context->entries[upper].background,
                    (int)(blend * 256.0f));
    }

    image_darken(context->screen, 60);
}

static void draw_page_dots(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float radius = screen->height * 0.006f;
    float spacing = screen->height * 0.03f;
    float start = screen->width / 2 - (context->entry_count - 1) * spacing / 2;
    int i;

    for (i = 0; i < context->entry_count; i++) {
        float weight = clampf(1.0f - absf(i - frame->position), 0.0f, 1.0f);
        int dot_alpha = (int)(alpha * (0.35f + 0.65f * weight));

        draw_round_rect(screen, start + i * spacing - radius, screen->height * 0.9f - radius,
                        radius * 2, radius * 2, radius, 0, COLOR_WHITE, dot_alpha);
    }
}

static void draw_centered(Image *screen, Face *face, int baseline, const char *text,
                          UINT32 color, int alpha)
{
    float width = text_width(face, text);

    draw_text_shadow(screen, face, (screen->width - width) / 2, baseline, text, color, alpha);
}

static void draw_buttons(Image *screen, const Entry *entry, const ThemeFrame *frame, int alpha)
{
    float height = (float)screen->height;
    float gap = height * 0.02f;
    float focus = clampf(frame->focus, 0.0f, 1.0f);
    int has_kernels = entry_has_kernels(entry);
    UiButton primary;
    UiButton advanced;
    float width;
    float primary_end;

    primary.y = height * 0.62f;
    primary.height = height * 0.064f;
    primary.label = entry_action_label(entry);
    primary.play_icon = 1;
    primary.active = 1.0f - focus;
    primary.idle_fill = 40;
    primary.border = 256;
    primary.fill_color = COLOR_WHITE;
    primary.text_color = COLOR_INK;

    advanced = primary;
    advanced.label = "Advanced options";
    advanced.play_icon = 0;
    advanced.active = focus;
    advanced.border = 150;

    width = ui_button_width(screen, &faces.button, &primary);
    if (has_kernels) {
        width += gap + ui_button_width(screen, &faces.button, &advanced);
    }

    primary.x = (screen->width - width) / 2;
    primary_end = ui_button_draw(screen, &faces.button, &primary, alpha);

    if (has_kernels) {
        advanced.x = primary_end + gap;
        ui_button_draw(screen, &faces.button, &advanced, alpha);
    }
}

static void draw_entry(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    Entry *entry = &context->entries[frame->selected];
    float height = (float)screen->height;
    float list = clampf(frame->list, 0.0f, 1.0f);
    int content_alpha = (int)(alpha * (1.0f - list));
    char subtitle[128];
    int logo_size = (int)(height * 0.14f);

    if (entry_has_logo(entry)) {
        draw_image(screen, &entry->logo, (screen->width - logo_size) / 2, (int)(height * 0.2f),
                   logo_size, logo_size, alpha);
    }

    draw_centered(screen, &faces.title, (int)(height * 0.5f), entry->title, COLOR_WHITE, alpha);

    entry_subtitle(entry, frame->selected == context->default_index, subtitle, sizeof(subtitle));
    draw_centered(screen, &faces.subtitle, (int)(height * 0.56f), subtitle, 0xe8e4ee,
                  content_alpha);

    draw_buttons(screen, entry, frame, content_alpha);

    if (list > 0.0f && entry_has_kernels(entry)) {
        UiOptionList options;

        options.heading_face = &faces.hint;
        options.row_face = &faces.list;
        options.heading = "ADVANCED OPTIONS";
        options.width = screen->width * 0.4f;
        options.x = (screen->width - options.width) / 2;
        options.top = height * 0.58f;
        options.row_height = height * 0.056f;
        options.rows = MINIMAL_LIST_ROWS;
        options.heading_color = 0xd0c8dc;
        options.text_color = COLOR_WHITE;
        options.highlight_color = COLOR_WHITE;
        options.accent_color = UI_ENTRY_ACCENT;
        options.shadow = 1;

        ui_option_list_draw(screen, &options, entry, frame, (int)(alpha * list));
    }

    if (frame->status[0] != '\0') {
        draw_centered(screen, &faces.hint, (int)(height * 0.78f), frame->status, COLOR_WHITE,
                      content_alpha);
    }
}

static void draw_hints(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    UiHintStyle hints = { &faces.hint, 0xe8e4ee, 1 };

    kit_hints_center(context->screen, &hints, (int)(context->screen->height * 0.955f),
                     &context->entries[frame->selected], frame, 0, alpha * 200 / 256);
}

static void minimal_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    float intro = ease_out_cubic(frame->intro_elapsed / AARAMBH_INTRO_MS);
    float launch = 0.0f;
    int alpha;
    UiClock clock;

    if (frame->launching) {
        launch = ease_in_out_cubic(frame->launch_elapsed / AARAMBH_LAUNCH_MS);
    }
    alpha = (int)(intro * (1.0f - launch) * 256.0f);

    draw_background(context, frame);
    draw_entry(context, frame, alpha);
    draw_page_dots(context, frame, alpha);
    draw_hints(context, frame, alpha);

    clock.time_face = &faces.clock;
    clock.period_face = &faces.clock_period;
    clock.right = screen->width - screen->height * 0.06f;
    clock.baseline = (int)(screen->height * 0.09f);
    clock.gap = screen->height * 0.008f;
    clock.time_color = COLOR_WHITE;
    clock.period_color = 0xebebf0;
    clock.shadow = 1;
    ui_clock_draw(screen, &clock, frame, alpha);

    if (frame->launching) {
        UiSplash splash;

        splash.title_face = &faces.splash;
        splash.detail_face = &faces.subtitle;
        splash.backdrop = NULL;
        splash.title_color = COLOR_WHITE;
        splash.detail_color = 0xe8e4ee;
        splash.dot_color = COLOR_WHITE;
        splash.darken = 100;
        splash.shadow = 1;
        ui_launch_splash_draw(context, frame, &splash);
    }

    if (intro < 1.0f) {
        image_darken(screen, (int)((1.0f - intro) * 256.0f));
    }
}

static const EditColors minimal_edit = { 0x1b1a20, 0x111014, UI_ENTRY_ACCENT, 0xffffff, 0xb3b0ba, EDIT_FRAME_CARD };

const Theme minimal_theme = {
    .name = "minimal",
    .list_rows = MINIMAL_LIST_ROWS,
    .edit = &minimal_edit,
    .load = minimal_load,
    .render = minimal_render
};