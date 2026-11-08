#include "aarambh/theme_kit.h"

#include "lib/math.h"

#define DIAL_COPPER  0xe08e6d
#define DIAL_BLUSH   0xf3c9b4
#define DIAL_IVORY   0xf6ece4
#define DIAL_SOFT    0xe6d8cf
#define DIAL_FACE    0x0b211b
#define DIAL_TITLES  3
#define DIAL_TICKS   60

typedef struct {
    Face titles[DIAL_TITLES];
    Face subtitle;
    Face button;
    Face brand;
    Face hint;
    Face clock;
    Face clock_period;
    Face list;
    Face letter;
    Face splash;
    Image backdrop;
    Image plain;
    Image glow;
} DialTheme;

static DialTheme dial;

static float dial_x(const Image *screen)
{
    return kit_x(screen, 806);
}

static float dial_y(const Image *screen)
{
    return screen->height / 2.0f;
}

static float dial_radius(const Image *screen)
{
    return kit_s(screen, 285);
}

static float slot_angle(int index, float count)
{
    return (float)(index * 2.0 * MATH_PI / count - MATH_PI / 2.0);
}

static void paint_dial(Image *backdrop, const Image *screen)
{
    float cx = dial_x(screen);
    float cy = dial_y(screen);
    float radius = dial_radius(screen);
    int tick;

    draw_ring(backdrop, cx, cy, radius + kit_s(screen, 14), kit_s(screen, 4), DIAL_COPPER, 200);
    draw_round_rect(backdrop, cx - radius, cy - radius, radius * 2, radius * 2, radius, 0,
                    DIAL_FACE, 256);
    draw_ring(backdrop, cx, cy, radius, 1, DIAL_COPPER, 90);

    for (tick = 0; tick < DIAL_TICKS; tick++) {
        float angle = (float)(tick * 2.0 * MATH_PI / DIAL_TICKS - MATH_PI / 2.0);
        float inner = radius - kit_s(screen, 10);
        float outer = radius - kit_s(screen, 4);
        float cosine = (float)math_cos(angle);
        float sine = (float)math_sin(angle);
        float thickness = 1.0f;
        int alpha = 90;

        if (tick % 12 == 0) {
            inner = radius - kit_s(screen, 22);
            thickness = kit_s(screen, 3);
            alpha = 230;
        }

        draw_line(backdrop, cx + inner * cosine, cy + inner * sine, cx + outer * cosine,
                  cy + outer * sine, thickness, DIAL_COPPER, alpha);
    }

    kit_text_center(backdrop, &dial.brand, cx, (int)(cy + kit_s(screen, 90) + dial.brand.ascent),
                    "AARAMBH", DIAL_COPPER, 200);
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &dial.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x0f2e25, 0x06130f)) {
        return;
    }

    kit_glow(backdrop, &dial.glow, dial_x(screen), dial_y(screen), kit_s(screen, 1000),
             kit_s(screen, 1000), 0x1f5c49, 140);
    image_noise(backdrop, 5, 0xd1a1);

    if (image_alloc(&dial.plain, screen->width, screen->height)) {
        image_copy(&dial.plain, backdrop);
    }

    paint_dial(backdrop, screen);
}

static void dial_load(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&dial.titles[0], context->font_bold, kit_s(screen, 86));
    face_init(&dial.titles[1], context->font_bold, kit_s(screen, 64));
    face_init(&dial.titles[2], context->font_bold, kit_s(screen, 46));
    face_init(&dial.subtitle, context->font_regular, kit_s(screen, 20));
    face_init(&dial.button, context->font_bold, kit_s(screen, 20));
    face_init(&dial.brand, context->font_bold, kit_s(screen, 14));
    face_init(&dial.hint, context->font_regular, kit_s(screen, 15));
    face_init(&dial.clock, context->font_bold, kit_s(screen, 40));
    face_init(&dial.clock_period, context->font_bold, kit_s(screen, 17));
    face_init(&dial.list, context->font_regular, kit_s(screen, 19));
    face_init(&dial.letter, context->font_bold, kit_s(screen, 26));
    face_init(&dial.splash, context->font_bold, kit_s(screen, 52));

    kit_glow_sprite(&dial.glow);
    build_backdrop(screen);
}

static void draw_slots(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float cx = dial_x(screen);
    float cy = dial_y(screen);
    float orbit = dial_radius(screen) - kit_s(screen, 82);
    int i;

    for (i = 0; i < context->entry_count; i++) {
        float angle = slot_angle(i, (float)context->entry_count);
        float weight = kit_weight(frame, i);
        float size = kit_s(screen, 40) + kit_s(screen, 22) * weight;
        float x = cx + orbit * (float)math_cos(angle);
        float y = cy + orbit * (float)math_sin(angle);

        kit_logo(screen, &context->entries[i], &dial.letter, x - size / 2, y - size / 2, size,
                 DIAL_SOFT, (int)(alpha * 0.55f * (1.0f - weight)));
        kit_logo(screen, &context->entries[i], &dial.letter, x - size / 2, y - size / 2, size,
                 DIAL_BLUSH, (int)(alpha * weight));
    }
}

static void draw_needle(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float cx = dial_x(screen);
    float cy = dial_y(screen);
    float angle = (float)(frame->position * 2.0 * MATH_PI / context->entry_count - MATH_PI / 2.0);
    float reach = dial_radius(screen) - kit_s(screen, 128);
    float tail = kit_s(screen, 30);
    float cosine = (float)math_cos(angle);
    float sine = (float)math_sin(angle);
    float hub = kit_s(screen, 15);
    float pin = kit_s(screen, 5);

    draw_line(screen, cx - tail * cosine, cy - tail * sine, cx + reach * cosine, cy + reach * sine,
              kit_s(screen, 6), DIAL_COPPER, alpha);
    draw_round_rect(screen, cx - hub, cy - hub, hub * 2, hub * 2, hub, 0, DIAL_COPPER, alpha);
    draw_round_rect(screen, cx - pin, cy - pin, pin * 2, pin * 2, pin, 0, DIAL_FACE, alpha);
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
    Face *title = kit_fit(dial.titles, DIAL_TITLES, entry->title, kit_x(screen, 430));
    char line[128];
    UiButton start;

    kit_detail_line(line, sizeof(line), context, frame, index, 0);

    draw_text(screen, title, left, (int)(kit_y(screen, 270) + dial.titles[0].ascent),
              entry->title, DIAL_IVORY, shown);
    draw_text(screen, &dial.subtitle, left + kit_s(screen, 4),
              (int)(kit_y(screen, 378) + dial.subtitle.ascent), line, DIAL_SOFT, content);

    start.x = left + kit_s(screen, 4);
    start.y = kit_y(screen, 428);
    start.height = kit_s(screen, 48);
    start.label = entry_action_label(entry);
    start.play_icon = 1;
    start.active = 1.0f;
    start.idle_fill = 0;
    start.border = 256;
    start.fill_color = DIAL_COPPER;
    start.text_color = DIAL_FACE;
    ui_button_draw(screen, &dial.button, &start, content);

    if (frame->status[0] != '\0') {
        draw_text(screen, &dial.hint, left + kit_s(screen, 4),
                  (int)(kit_y(screen, 512) + dial.hint.ascent), frame->status, DIAL_COPPER,
                  content * 200 / 256);
    }

    kit_kernel_list(screen, entry, frame, &dial.hint, &dial.list, left, kit_y(screen, 400),
                    kit_x(screen, 420), DIAL_IVORY, DIAL_COPPER, shown);
}

static void dial_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &dial.hint, DIAL_SOFT, 0 };
    UiClock clock;

    kit_draw_backdrop(screen, &dial.backdrop);

    draw_slots(context, frame, fade.alpha);
    draw_needle(context, frame, fade.alpha);
    draw_details(context, frame, fade.alpha);

    kit_hints(screen, &hints, kit_x(screen, 84), (int)(screen->height - kit_s(screen, 40)),
              &context->entries[frame->selected], frame, 0, fade.alpha * 200 / 256);

    clock.time_face = &dial.clock;
    clock.period_face = &dial.clock_period;
    clock.right = screen->width - kit_x(screen, 48);
    clock.baseline = (int)(kit_y(screen, 28) + dial.clock.ascent);
    clock.gap = kit_s(screen, 6);
    clock.time_color = DIAL_IVORY;
    clock.period_color = DIAL_IVORY;
    clock.shadow = 0;
    ui_clock_draw(screen, &clock, frame, fade.alpha);

    kit_splash(context, frame, &dial.splash, &dial.subtitle, &dial.plain, DIAL_IVORY,
               DIAL_COPPER);
    kit_intro_blackout(screen, frame);
}

static const EditColors dial_edit = { 0x0f2a22, 0x081a15, 0xe08e6d, 0xf6ece4, 0xb7a89e, EDIT_FRAME_CARD };

const Theme dial_theme = {
    .name = "dial",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 0,
    .down_opens_list = 1,
    .edit = &dial_edit,
    .load = dial_load,
    .render = dial_render
};