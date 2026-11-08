#include "aarambh/theme_kit.h"

#define STAR_BLUE    0x9db7ff
#define STAR_PALE    0xc7d2ff
#define STAR_TEXT    0xeef1ff
#define STAR_LINE    0xc7d2ff
#define STAR_TITLES  3
#define STAR_HEIGHTS 7

typedef struct {
    Face titles[STAR_TITLES];
    Face detail;
    Face name;
    Face name_selected;
    Face hint;
    Face clock;
    Face clock_period;
    Face list;
    Face splash;
    Image backdrop;
    Image glow;
} ConstellationTheme;

static ConstellationTheme constellation;

static const float star_heights[STAR_HEIGHTS] = { 440, 300, 200, 340, 170, 380, 250 };

static Point star_position(const Image *screen, int index, int count)
{
    float left = 220.0f;
    float right = 1070.0f;
    Point point;

    if (count > 1) {
        point.x = kit_x(screen, left + (right - left) * index / (count - 1));
    } else {
        point.x = kit_x(screen, 640);
    }

    point.y = kit_y(screen, star_heights[index % STAR_HEIGHTS]);
    return point;
}

static void build_backdrop(const ThemeContext *context)
{
    const Image *screen = context->screen;
    Image *backdrop = &constellation.backdrop;
    int i;

    if (!kit_backdrop(backdrop, screen, 0x05061a, 0x140f3d)) {
        return;
    }

    kit_glow(backdrop, &constellation.glow, kit_x(screen, 720), kit_y(screen, 120),
             kit_s(screen, 1300), kit_s(screen, 1000), 0x3b2f9e, 120);
    kit_glow(backdrop, &constellation.glow, kit_x(screen, 1150), kit_y(screen, 600),
             kit_s(screen, 900), kit_s(screen, 800), 0x1e3a8a, 100);
    kit_stars(backdrop, 320, 0x57a2, 0xdce4ff, 220, (float)backdrop->height);

    for (i = 0; i + 1 < context->entry_count; i++) {
        Point a = star_position(screen, i, context->entry_count);
        Point b = star_position(screen, i + 1, context->entry_count);

        draw_line(backdrop, a.x, a.y, b.x, b.y, kit_s(screen, 1.6f), STAR_LINE, 80);
    }
}

static void constellation_load(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&constellation.titles[0], context->font_bold, kit_s(screen, 66));
    face_init(&constellation.titles[1], context->font_bold, kit_s(screen, 52));
    face_init(&constellation.titles[2], context->font_bold, kit_s(screen, 40));
    face_init(&constellation.detail, context->font_regular, kit_s(screen, 17));
    face_init(&constellation.name, context->font_regular, kit_s(screen, 14));
    face_init(&constellation.name_selected, context->font_bold, kit_s(screen, 22));
    face_init(&constellation.hint, context->font_regular, kit_s(screen, 15));
    face_init(&constellation.clock, context->font_bold, kit_s(screen, 40));
    face_init(&constellation.clock_period, context->font_bold, kit_s(screen, 17));
    face_init(&constellation.list, context->font_regular, kit_s(screen, 19));
    face_init(&constellation.splash, context->font_bold, kit_s(screen, 50));

    kit_glow_sprite(&constellation.glow);
    build_backdrop(context);
}

static void draw_stars(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    int lower = (int)frame->position;
    float fraction = frame->position - lower;
    Point light = star_position(screen, lower, context->entry_count);
    int i;

    if (lower + 1 < context->entry_count) {
        Point next = star_position(screen, lower + 1, context->entry_count);

        light.x += (next.x - light.x) * fraction;
        light.y += (next.y - light.y) * fraction;
    }

    kit_glow(screen, &constellation.glow, light.x, light.y, kit_s(screen, 220), kit_s(screen, 220),
             STAR_BLUE, alpha * 170 / 256);

    for (i = 0; i < context->entry_count; i++) {
        const Entry *entry = &context->entries[i];
        Point point = star_position(screen, i, context->entry_count);
        float weight = kit_weight(frame, i);
        float size = kit_s(screen, 38) + kit_s(screen, 48) * weight;
        Face *face = &constellation.name;
        float gap = kit_s(screen, 32) + kit_s(screen, 30) * weight;

        if (weight > 0.5f) {
            face = &constellation.name_selected;
        }

        kit_glow(screen, &constellation.glow, point.x, point.y, kit_s(screen, 120),
                 kit_s(screen, 120), STAR_BLUE, alpha * 50 / 256);
        kit_logo(screen, entry, &constellation.name_selected, point.x - size / 2,
                 point.y - size / 2, size, STAR_TEXT, (int)(alpha * (0.7f + 0.3f * weight)));
        kit_text_center(screen, face, point.x, (int)(point.y + gap + face->ascent),
                        entry->title, STAR_TEXT, (int)(alpha * (0.58f + 0.42f * weight)));
    }
}

static void draw_caption(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    int shown = (int)(alpha * visibility);
    int content = (int)(shown * (1.0f - frame->list));
    Face *title = kit_fit(constellation.titles, STAR_TITLES, entry->title, kit_x(screen, 700));
    char line[160];

    kit_detail_line(line, sizeof(line), context, frame, index, 1);

    draw_text(screen, title, kit_x(screen, 80),
              (int)(screen->height - kit_y(screen, 160) + constellation.titles[0].ascent),
              entry->title, STAR_TEXT, content);
    draw_text(screen, &constellation.detail, kit_x(screen, 84),
              (int)(screen->height - kit_y(screen, 76) + constellation.detail.ascent), line,
              STAR_PALE, content * 200 / 256);

    kit_kernel_list(screen, entry, frame, &constellation.hint, &constellation.list,
                    screen->width - kit_x(screen, 600), screen->height - kit_y(screen, 290),
                    kit_x(screen, 520),
                    STAR_TEXT, STAR_BLUE, shown);
}

static void constellation_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &constellation.hint, STAR_PALE, 0 };
    const Entry *entry = &context->entries[frame->selected];
    float width = kit_hints_width(&hints, entry, frame, 0);
    UiClock clock;

    kit_draw_backdrop(screen, &constellation.backdrop);

    draw_stars(context, frame, fade.alpha);
    draw_caption(context, frame, fade.alpha);

    kit_hints(screen, &hints, screen->width - kit_x(screen, 48) - width,
              (int)(screen->height - kit_s(screen, 40)), entry, frame, 0, fade.alpha * 200 / 256);

    clock.time_face = &constellation.clock;
    clock.period_face = &constellation.clock_period;
    clock.right = screen->width - kit_x(screen, 48);
    clock.baseline = (int)(kit_y(screen, 28) + constellation.clock.ascent);
    clock.gap = kit_s(screen, 6);
    clock.time_color = STAR_TEXT;
    clock.period_color = STAR_TEXT;
    clock.shadow = 0;
    ui_clock_draw(screen, &clock, frame, fade.alpha);

    kit_splash(context, frame, &constellation.splash, &constellation.detail,
               &constellation.backdrop, STAR_TEXT, STAR_PALE);
    kit_intro_blackout(screen, frame);
}

static const EditColors constellation_edit = { 0x12133a, 0x0a0b26, 0x9db7ff, 0xeef1ff, 0x9ea6d6, EDIT_FRAME_CARD };

const Theme constellation_theme = {
    .name = "constellation",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 0,
    .down_opens_list = 1,
    .edit = &constellation_edit,
    .load = constellation_load,
    .render = constellation_render
};