#include "aarambh/theme_kit.h"

#define GLASS_ICE      0x5ee0d4
#define GLASS_INK      0x06202a
#define GLASS_TEXT     0xf0fcff
#define GLASS_SOFT     0xc8ebf5
#define GLASS_TINT     0xdcf5ff
#define GLASS_ROW_SIZE 84.0f
#define GLASS_TITLES   3

typedef struct {
    Face label;
    Face row_title;
    Face row_detail;
    Face titles[GLASS_TITLES];
    Face subtitle;
    Face button;
    Face hint;
    Face clock;
    Face clock_period;
    Face splash;
    Face list;
    Image backdrop;
    Image frosted;
    Image glow;
} GlassTheme;

static GlassTheme glass;

static float sidebar_width(const Image *screen)
{
    return kit_x(screen, 390);
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &glass.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x071624, 0x0f2c44)) {
        return;
    }

    kit_glow(backdrop, &glass.glow, kit_x(screen, 1100), kit_y(screen, 150),
             kit_s(screen, 1000), kit_s(screen, 900), 0x2a8fbf, 150);
    kit_glow(backdrop, &glass.glow, kit_x(screen, 750), kit_y(screen, 775),
             kit_s(screen, 900), kit_s(screen, 850), GLASS_ICE, 90);
    kit_glow(backdrop, &glass.glow, kit_x(screen, 100), kit_y(screen, 200),
             kit_s(screen, 800), kit_s(screen, 800), 0x7aa7ff, 70);

    if (image_alloc(&glass.frosted, screen->width, screen->height)) {
        image_copy(&glass.frosted, backdrop);
        image_blur(&glass.frosted, (int)kit_s(screen, 20));
    }
}

static void glass_load(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&glass.label, context->font_bold, kit_s(screen, 15));
    face_init(&glass.row_title, context->font_bold, kit_s(screen, 21));
    face_init(&glass.row_detail, context->font_regular, kit_s(screen, 13));
    face_init(&glass.titles[0], context->font_bold, kit_s(screen, 100));
    face_init(&glass.titles[1], context->font_bold, kit_s(screen, 72));
    face_init(&glass.titles[2], context->font_bold, kit_s(screen, 52));
    face_init(&glass.subtitle, context->font_regular, kit_s(screen, 21));
    face_init(&glass.button, context->font_bold, kit_s(screen, 19));
    face_init(&glass.hint, context->font_regular, kit_s(screen, 13));
    face_init(&glass.clock, context->font_bold, kit_s(screen, 40));
    face_init(&glass.clock_period, context->font_bold, kit_s(screen, 17));
    face_init(&glass.splash, context->font_bold, kit_s(screen, 48));
    face_init(&glass.list, context->font_regular, kit_s(screen, 21));

    kit_glow_sprite(&glass.glow);
    build_backdrop(screen);
}

static float row_top(const ThemeContext *context, int index)
{
    const Image *screen = context->screen;
    float top = kit_y(screen, 108) + index * kit_s(screen, GLASS_ROW_SIZE);
    int i;

    for (i = 0; i < index; i++) {
        if (entry_is_system(&context->entries[i]) && !entry_is_system(&context->entries[i + 1])) {
            top += kit_s(screen, 26);
        }
    }

    return top;
}

static void draw_sidebar_frame(Image *screen, int alpha)
{
    float width = sidebar_width(screen);

    draw_backdrop(screen, &glass.frosted, 0, 0, width, (float)screen->height, 0, alpha);
    draw_rect(screen, 0, 0, (int)width, screen->height, GLASS_TINT, alpha * 26 / 256);
    draw_rect(screen, (int)width - 1, 0, 1, screen->height, GLASS_SOFT, alpha * 60 / 256);
    draw_text(screen, &glass.label, kit_x(screen, 40),
              (int)(kit_y(screen, 46) + glass.label.ascent),
              "AARAMBH", GLASS_ICE, alpha * 220 / 256);
}

static void draw_highlight(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    int lower = (int)frame->position;
    float fraction = frame->position - lower;
    float top = row_top(context, lower);
    float right = sidebar_width(screen) - kit_s(screen, 20);
    float height = kit_s(screen, 76);

    if (lower + 1 < context->entry_count) {
        top += (row_top(context, lower + 1) - top) * fraction;
    }

    draw_round_rect(screen, kit_s(screen, 20), top, right - kit_s(screen, 20), height,
                    kit_s(screen, 18), 0, GLASS_TINT, alpha * 46 / 256);
    draw_round_rect(screen, kit_s(screen, 20), top, right - kit_s(screen, 20), height,
                    kit_s(screen, 18), 1.2f, GLASS_TINT, alpha * 90 / 256);
    draw_rect(screen, (int)kit_s(screen, 22), (int)(top + kit_s(screen, 20)),
              (int)kit_s(screen, 4), (int)kit_s(screen, 36), GLASS_ICE, alpha);
}

static void draw_rows(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    int i;

    for (i = 0; i < context->entry_count; i++) {
        const Entry *entry = &context->entries[i];
        float weight = kit_weight(frame, i);
        float top = row_top(context, i);
        int row_alpha = (int)(alpha * (0.65f + 0.35f * weight));
        const char *detail = entry_launch_detail(entry, -1);
        char subtitle[96];

        if (i > 0 && entry_is_system(&context->entries[i - 1]) && !entry_is_system(entry)) {
            draw_text(screen, &glass.hint, kit_x(screen, 40), (int)(top - kit_s(screen, 8)),
                      "SYSTEM", GLASS_SOFT, alpha * 130 / 256);
        }

        if (detail[0] == '\0') {
            entry_subtitle(entry, 0, subtitle, sizeof(subtitle));
            detail = subtitle;
        }

        kit_logo(screen, entry, &glass.row_title, kit_x(screen, 44), top + kit_s(screen, 19),
                 kit_s(screen, 38), GLASS_TEXT, row_alpha);
        draw_text(screen, &glass.row_title, kit_x(screen, 102),
                  (int)(top + kit_s(screen, 15) + glass.row_title.ascent), entry->title,
                  GLASS_TEXT, row_alpha);
        draw_text(screen, &glass.row_detail, kit_x(screen, 102),
                  (int)(top + kit_s(screen, 45) + glass.row_detail.ascent), detail, GLASS_SOFT,
                  row_alpha * 170 / 256);
    }
}

static void draw_buttons(Image *screen, const Entry *entry, float x, float y, int alpha)
{
    UiButton start;
    float end;

    start.x = x;
    start.y = y;
    start.height = kit_s(screen, 48);
    start.label = entry_action_label(entry);
    start.play_icon = 1;
    start.active = 1.0f;
    start.idle_fill = 0;
    start.border = 256;
    start.fill_color = GLASS_ICE;
    start.text_color = GLASS_INK;
    end = ui_button_draw(screen, &glass.button, &start, alpha);

    if (entry_has_kernels(entry)) {
        UiButton options = start;

        options.x = end + kit_s(screen, 16);
        options.label = "Options";
        options.play_icon = 0;
        options.active = 0.0f;
        options.idle_fill = 30;
        options.border = 120;
        options.fill_color = GLASS_TEXT;
        ui_button_draw(screen, &glass.button, &options, alpha);
    }
}

static void draw_detail(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    float left = sidebar_width(screen) + kit_x(screen, 90);
    float slide = (1.0f - visibility) * kit_s(screen, 24);
    int shown = (int)(alpha * visibility);
    int content = (int)(shown * (1.0f - frame->list));
    float max_width = screen->width - left - kit_x(screen, 60);
    Face *title = kit_fit(glass.titles, GLASS_TITLES, entry->title, max_width);
    char line[128];

    kit_logo(screen, entry, &glass.titles[2], left, kit_y(screen, 220) - slide,
             kit_s(screen, 112), GLASS_TEXT, shown);
    draw_text(screen, title, left - kit_s(screen, 6),
              (int)(kit_y(screen, 350) + glass.titles[0].ascent - slide),
              entry->title, GLASS_TEXT, shown);

    kit_detail_line(line, sizeof(line), context, frame, index, 0);
    draw_text(screen, &glass.subtitle, left, (int)(kit_y(screen, 475) + glass.subtitle.ascent),
              line, GLASS_SOFT, content * 220 / 256);

    draw_buttons(screen, entry, left, kit_y(screen, 530), content);

    if (frame->status[0] != '\0') {
        draw_text(screen, &glass.hint, left, (int)(kit_y(screen, 610) + glass.hint.ascent),
                  frame->status, GLASS_SOFT, content * 180 / 256);
    }

    kit_kernel_list(screen, entry, frame, &glass.hint, &glass.list, left, kit_y(screen, 530),
                    max_width, GLASS_TEXT, GLASS_ICE, shown);
}

static void glass_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &glass.hint, GLASS_SOFT, 0 };
    UiClock clock;

    kit_draw_backdrop(screen, &glass.backdrop);

    draw_sidebar_frame(screen, fade.alpha);
    draw_highlight(context, frame, fade.alpha);
    draw_rows(context, frame, fade.alpha);
    draw_detail(context, frame, fade.alpha);

    kit_hints(screen, &hints, kit_x(screen, 40), (int)(screen->height - kit_s(screen, 34)),
              &context->entries[frame->selected], frame, 1, fade.alpha * 180 / 256);

    clock.time_face = &glass.clock;
    clock.period_face = &glass.clock_period;
    clock.right = screen->width - kit_x(screen, 48);
    clock.baseline = (int)(kit_y(screen, 28) + glass.clock.ascent);
    clock.gap = kit_s(screen, 6);
    clock.time_color = GLASS_TEXT;
    clock.period_color = GLASS_TEXT;
    clock.shadow = 0;
    ui_clock_draw(screen, &clock, frame, fade.alpha);

    kit_splash(context, frame, &glass.splash, &glass.subtitle, &glass.backdrop, GLASS_TEXT,
               GLASS_SOFT);
    kit_intro_blackout(screen, frame);
}

static const EditColors glass_edit = { 0x0a2633, 0x061a23, 0x5ee0d4, 0xf0fcff, 0x9cc7d3, EDIT_FRAME_CARD };

const Theme glass_sidebar_theme = {
    .name = "glass-sidebar",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 1,
    .down_opens_list = 0,
    .edit = &glass_edit,
    .load = glass_load,
    .render = glass_render
};