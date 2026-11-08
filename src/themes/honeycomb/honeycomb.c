#include "aarambh/theme_kit.h"

#include "lib/math.h"
#include "lib/str.h"

#define HONEY        0xffb627
#define HONEY_LIGHT  0xffcf6e
#define HONEY_PALE   0xffd98a
#define HONEY_CREAM  0xfff3d6
#define HONEY_DARK   0x2a1a02
#define HONEY_CELL   0x2a1f0b
#define HONEY_TITLES 3
#define HONEY_SLOTS  19

typedef struct {
    float column;
    float row;
} HexSlot;

typedef struct {
    Face titles[HONEY_TITLES];
    Face subtitle;
    Face cell;
    Face button;
    Face hint;
    Face clock;
    Face clock_period;
    Face list;
    Face splash;
    Image backdrop;
    Image glow;
} HoneycombTheme;

static HoneycombTheme honeycomb;
static float cell_scale = 1.0f;

static const HexSlot hex_slots[HONEY_SLOTS] = {
    { 0, 0 }, { 1, 0 }, { 0.5f, 1 }, { -0.5f, 1 }, { -1, 0 }, { -0.5f, -1 }, { 0.5f, -1 },
    { 2, 0 }, { 1.5f, 1 }, { 1, 2 }, { 0, 2 }, { -1, 2 }, { -1.5f, 1 }, { -2, 0 },
    { -1.5f, -1 }, { -1, -2 }, { 0, -2 }, { 1, -2 }, { 1.5f, -1 }
};

static float cell_radius(const Image *screen)
{
    return kit_s(screen, 118) * cell_scale;
}

static Point slot_center(const Image *screen, int index)
{
    float radius = cell_radius(screen);
    float step = radius * (float)math_sqrt(3.0);
    Point center;

    center.x = kit_x(screen, 820) + hex_slots[index].column * step;
    center.y = screen->height * 0.5f + hex_slots[index].row * radius * 1.5f;
    return center;
}

static void hexagon(Point *points, float center_x, float center_y, float radius)
{
    int i;

    for (i = 0; i < 6; i++) {
        float angle = (float)((60.0 * i - 30.0) * MATH_PI / 180.0);

        points[i].x = center_x + radius * (float)math_cos(angle);
        points[i].y = center_y + radius * (float)math_sin(angle);
    }
}

static void hexagon_outline(Image *destination, float center_x, float center_y, float radius,
                            float thickness, UINT32 color, int alpha)
{
    Point points[6];
    int i;

    hexagon(points, center_x, center_y, radius);
    for (i = 0; i < 6; i++) {
        const Point *a = &points[i];
        const Point *b = &points[(i + 1) % 6];

        draw_line(destination, a->x, a->y, b->x, b->y, thickness, color, alpha);
    }
}

static void paint_pattern(Image *backdrop, const Image *screen)
{
    float radius = kit_s(screen, 46);
    float step = radius * (float)math_sqrt(3.0);
    int row;
    int column;

    for (row = -1; row * radius * 1.5f < screen->height + radius; row++) {
        for (column = -1; column * step < screen->width + step; column++) {
            float x = column * step;

            if (row % 2 != 0) {
                x += step / 2;
            }

            hexagon_outline(backdrop, x, row * radius * 1.5f, radius - 3, 1.0f, HONEY, 18);
        }
    }
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &honeycomb.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x1c1407, 0x0c0903)) {
        return;
    }

    kit_glow(backdrop, &honeycomb.glow, kit_x(screen, 900), kit_y(screen, 450),
             kit_s(screen, 1100), kit_s(screen, 1000), 0xff9f1c, 70);
    paint_pattern(backdrop, screen);
}

static void honeycomb_load(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&honeycomb.titles[0], context->font_bold, kit_s(screen, 84));
    face_init(&honeycomb.titles[1], context->font_bold, kit_s(screen, 62));
    face_init(&honeycomb.titles[2], context->font_bold, kit_s(screen, 44));
    face_init(&honeycomb.subtitle, context->font_regular, kit_s(screen, 20));
    face_init(&honeycomb.cell, context->font_bold, kit_s(screen, 20));
    face_init(&honeycomb.button, context->font_bold, kit_s(screen, 20));
    face_init(&honeycomb.hint, context->font_regular, kit_s(screen, 15));
    face_init(&honeycomb.clock, context->font_bold, kit_s(screen, 40));
    face_init(&honeycomb.clock_period, context->font_bold, kit_s(screen, 17));
    face_init(&honeycomb.list, context->font_regular, kit_s(screen, 19));
    face_init(&honeycomb.splash, context->font_bold, kit_s(screen, 52));

    kit_glow_sprite(&honeycomb.glow);
    build_backdrop(screen);
}

static Point selection_center(const ThemeContext *context, const ThemeFrame *frame)
{
    int lower = (int)frame->position;
    float fraction = frame->position - lower;
    Point from = slot_center(context->screen, lower);
    Point to = from;

    if (lower + 1 < context->entry_count) {
        to = slot_center(context->screen, lower + 1);
    }

    from.x += (to.x - from.x) * fraction;
    from.y += (to.y - from.y) * fraction;
    return from;
}

static void draw_cells(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float radius = cell_radius(screen);
    Point glow = selection_center(context, frame);
    int count = context->entry_count;
    int i;

    if (count > HONEY_SLOTS) {
        count = HONEY_SLOTS;
    }

    kit_glow(screen, &honeycomb.glow, glow.x, glow.y, radius * 3, radius * 3, HONEY,
             alpha * 150 / 256);

    for (i = 0; i < count; i++) {
        const Entry *entry = &context->entries[i];
        Point center = slot_center(screen, i);
        float weight = kit_weight(frame, i);
        float size = (kit_s(screen, 52) + kit_s(screen, 32) * weight) * cell_scale;
        float lift = kit_s(screen, 12) * weight * cell_scale;
        Point points[6];

        hexagon(points, center.x, center.y, radius - 6);
        draw_polygon(screen, points, 6, HONEY_CELL, alpha);
        draw_polygon(screen, points, 6, HONEY, (int)(alpha * weight));
        hexagon_outline(screen, center.x, center.y, radius - 6, 1.5f, HONEY_LIGHT,
                        (int)(alpha * (0.35f + 0.5f * weight)));

        kit_logo(screen, entry, &honeycomb.cell, center.x - size / 2, center.y - size / 2 - lift,
                 size, HONEY_PALE, (int)(alpha * (1.0f - weight) * 0.75f));
        kit_logo(screen, entry, &honeycomb.cell, center.x - size / 2, center.y - size / 2 - lift,
                 size, HONEY_DARK, (int)(alpha * weight));

        if (weight > 0.5f) {
            kit_text_center(screen, &honeycomb.cell, center.x,
                            (int)(center.y + kit_s(screen, 44) + honeycomb.cell.ascent),
                            entry->title, HONEY_DARK, (int)(alpha * (weight * 2 - 1)));
        }
    }
}

static void role_line(char *output, int capacity, const ThemeContext *context, int index)
{
    const Entry *entry = &context->entries[index];

    if (!entry_is_system(entry)) {
        entry_subtitle(entry, 0, output, capacity);
    } else if (index == context->default_index) {
        str_copy(output, "Default system", capacity);
    } else {
        str_copy(output, "System", capacity);
    }
}

static void draw_details(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    int shown = (int)(alpha * visibility);
    int content = (int)(shown * (1.0f - frame->list));
    float left = kit_x(screen, 80);
    Face *title = kit_fit(honeycomb.titles, HONEY_TITLES, entry->title, kit_x(screen, 420));
    char role[96];
    UiButton start;

    role_line(role, sizeof(role), context, index);

    draw_text(screen, title, left, (int)(kit_y(screen, 250) + honeycomb.titles[0].ascent),
              entry->title, HONEY_CREAM, shown);
    draw_text(screen, &honeycomb.subtitle, left + kit_s(screen, 4),
              (int)(kit_y(screen, 362) + honeycomb.subtitle.ascent), role, HONEY_PALE, content);
    draw_text(screen, &honeycomb.subtitle, left + kit_s(screen, 4),
              (int)(kit_y(screen, 392) + honeycomb.subtitle.ascent), entry_launch_detail(entry, -1),
              HONEY_PALE, content * 170 / 256);

    start.x = left + kit_s(screen, 4);
    start.y = kit_y(screen, 446);
    start.height = kit_s(screen, 48);
    start.label = entry_action_label(entry);
    start.play_icon = 1;
    start.active = 1.0f;
    start.idle_fill = 0;
    start.border = 256;
    start.fill_color = HONEY;
    start.text_color = HONEY_DARK;
    ui_button_draw(screen, &honeycomb.button, &start, content);

    if (frame->status[0] != '\0') {
        draw_text(screen, &honeycomb.hint, left + kit_s(screen, 4),
                  (int)(kit_y(screen, 530) + honeycomb.hint.ascent), frame->status, HONEY_PALE,
                  content * 180 / 256);
    }

    kit_kernel_list(screen, entry, frame, &honeycomb.hint, &honeycomb.list, left,
                    kit_y(screen, 380), kit_x(screen, 440), HONEY_CREAM, HONEY, shown);
}

static void honeycomb_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &honeycomb.hint, HONEY_PALE, 0 };
    UiClock clock;

    cell_scale = 1.0f;
    if (context->entry_count > 7) {
        cell_scale = 0.62f;
    }

    kit_draw_backdrop(screen, &honeycomb.backdrop);

    draw_cells(context, frame, fade.alpha);
    draw_details(context, frame, fade.alpha);

    kit_hints_center(screen, &hints, (int)(screen->height - kit_s(screen, 40)),
                     &context->entries[frame->selected], frame, 0, fade.alpha);

    clock.time_face = &honeycomb.clock;
    clock.period_face = &honeycomb.clock_period;
    clock.right = screen->width - kit_x(screen, 48);
    clock.baseline = (int)(kit_y(screen, 28) + honeycomb.clock.ascent);
    clock.gap = kit_s(screen, 6);
    clock.time_color = HONEY_CREAM;
    clock.period_color = HONEY_CREAM;
    clock.shadow = 0;
    ui_clock_draw(screen, &clock, frame, fade.alpha);

    kit_splash(context, frame, &honeycomb.splash, &honeycomb.subtitle, &honeycomb.backdrop,
               HONEY_CREAM, HONEY);
    kit_intro_blackout(screen, frame);
}

static const EditColors honeycomb_edit = { 0x2a1f0b, 0x1a1306, 0xffb627, 0xfff3d6, 0xcdb582, EDIT_FRAME_CARD };

const Theme honeycomb_theme = {
    .name = "honeycomb",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 0,
    .down_opens_list = 1,
    .edit = &honeycomb_edit,
    .load = honeycomb_load,
    .render = honeycomb_render
};