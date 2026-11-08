#include "aarambh/theme_kit.h"

#include "lib/str.h"

#define CINEMA_GOLD   0xf5b301
#define CINEMA_LIGHT  0xfff4dc
#define CINEMA_WARM   0xffe6be
#define CINEMA_PALE   0xffd27a
#define CINEMA_TILE   0x28160a
#define CINEMA_TITLES 3

typedef struct {
    Face label;
    Face titles[CINEMA_TITLES];
    Face detail;
    Face hint;
    Face clock;
    Face list;
    Face letter;
    Face splash;
    Image backdrop;
    Image glow;
} CinemaTheme;

static CinemaTheme cinema;

static float bar_height(const Image *screen)
{
    return kit_y(screen, 90);
}

static float strip_top(const Image *screen)
{
    return screen->height - kit_y(screen, 140);
}

static void draw_sparkles(Image *backdrop, const Image *screen)
{
    UINT32 state = 0xc1e3a;
    int i;

    for (i = 0; i < 14; i++) {
        float x = (float)(image_random(&state) % (UINT32)screen->width);
        float y = kit_y(screen, 100) + (float)(image_random(&state) % (UINT32)kit_y(screen, 500));

        kit_glow(backdrop, &cinema.glow, x, y, kit_s(screen, 18), kit_s(screen, 18),
                 CINEMA_PALE, 200);
    }
}

static void draw_letterbox(Image *backdrop, const Image *screen)
{
    float strip = strip_top(screen);
    int fade = (int)kit_s(screen, 240);
    int i;

    for (i = 0; i < fade; i++) {
        draw_rect(backdrop, 0, (int)strip - fade + i, screen->width, 1, COLOR_BLACK,
                  i * 256 / fade);
    }

    draw_rect(backdrop, 0, 0, screen->width, (int)bar_height(screen), COLOR_BLACK, 256);
    draw_rect(backdrop, 0, (int)strip, screen->width, screen->height, COLOR_BLACK, 256);
    draw_text(backdrop, &cinema.label, kit_x(screen, 48),
              (int)(kit_y(screen, 34) + cinema.label.ascent), "AARAMBH", CINEMA_GOLD, 256);
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &cinema.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x3a1d0c, 0x120806)) {
        return;
    }

    kit_glow(backdrop, &cinema.glow, kit_x(screen, 1000), kit_y(screen, 300),
             kit_s(screen, 1300), kit_s(screen, 1100), 0xff8a2a, 170);
    kit_glow(backdrop, &cinema.glow, kit_x(screen, 1100), kit_y(screen, 300),
             kit_s(screen, 550), kit_s(screen, 550), CINEMA_PALE, 140);
    draw_sparkles(backdrop, screen);
    image_noise(backdrop, 12, 0xf11a);
    draw_letterbox(backdrop, screen);
}

static void cinema_load(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&cinema.label, context->font_bold, kit_s(screen, 16));
    face_init(&cinema.titles[0], context->font_bold, kit_s(screen, 96));
    face_init(&cinema.titles[1], context->font_bold, kit_s(screen, 72));
    face_init(&cinema.titles[2], context->font_bold, kit_s(screen, 52));
    face_init(&cinema.detail, context->font_regular, kit_s(screen, 18));
    face_init(&cinema.hint, context->font_regular, kit_s(screen, 15));
    face_init(&cinema.clock, context->font_bold, kit_s(screen, 20));
    face_init(&cinema.list, context->font_regular, kit_s(screen, 20));
    face_init(&cinema.letter, context->font_bold, kit_s(screen, 30));
    face_init(&cinema.splash, context->font_bold, kit_s(screen, 52));

    kit_glow_sprite(&cinema.glow);
    build_backdrop(screen);
}

static void draw_feature(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    float strip = strip_top(screen);
    int shown = (int)(alpha * visibility);
    int content = (int)(shown * (1.0f - frame->list));
    float slide = (1.0f - visibility) * kit_s(screen, 40);
    Face *title = kit_fit(cinema.titles, CINEMA_TITLES, entry->title, kit_x(screen, 700));
    char line[160];

    kit_logo(screen, entry, &cinema.titles[0], kit_x(screen, 760) + slide, kit_y(screen, 140),
             kit_s(screen, 380), CINEMA_PALE, shown * 26 / 256);

    draw_text(screen, &cinema.label, kit_x(screen, 80),
              (int)(strip - kit_s(screen, 236) + cinema.label.ascent), "NOW SHOWING", CINEMA_GOLD,
              content);
    draw_text(screen, title, kit_x(screen, 76) - slide,
              (int)(strip - kit_s(screen, 206) + cinema.titles[0].ascent), entry->title,
              CINEMA_LIGHT, shown);

    str_copy(line, entry_launch_detail(entry, -1), sizeof(line));
    if (line[0] == '\0') {
        entry_subtitle(entry, index == context->default_index, line, sizeof(line));
    }
    if (frame->status[0] != '\0') {
        str_append(line, "   -   ", sizeof(line));
        str_append(line, frame->status, sizeof(line));
    }
    draw_text(screen, &cinema.detail, kit_x(screen, 82),
              (int)(strip - kit_s(screen, 86) + cinema.detail.ascent), line, CINEMA_WARM,
              content * 220 / 256);

    kit_kernel_list(screen, entry, frame, &cinema.label, &cinema.list, kit_x(screen, 80),
                    kit_y(screen, 170), kit_x(screen, 560), CINEMA_LIGHT, CINEMA_GOLD, shown);
}

/* Widest hint line over all entries, so the strip does not jump when one has no kernels. */
static float widest_hints(const ThemeContext *context, const ThemeFrame *frame,
                          const UiHintStyle *hints)
{
    float widest = 0.0f;
    int i;

    for (i = 0; i < context->entry_count; i++) {
        float width = kit_hints_width(hints, &context->entries[i], frame, 0);

        if (width > widest) {
            widest = width;
        }
    }

    return widest;
}

static void draw_filmstrip(const ThemeContext *context, const ThemeFrame *frame, float min_left,
                           int alpha)
{
    Image *screen = context->screen;
    float gap = kit_s(screen, 12);
    float max_total = screen->width - kit_x(screen, 80) - min_left;
    float width = kit_s(screen, 140);
    float height = kit_s(screen, 78);
    float top = strip_top(screen) + kit_s(screen, 34);
    float left;
    float outline;
    int i;

    if (max_total > kit_x(screen, 820)) {
        max_total = kit_x(screen, 820);
    }

    if (context->entry_count * (width + gap) - gap > max_total) {
        width = (max_total + gap) / context->entry_count - gap;
    }

    left = screen->width - kit_x(screen, 80) - context->entry_count * (width + gap) + gap;

    for (i = 0; i < context->entry_count; i++) {
        const Entry *entry = &context->entries[i];
        float weight = kit_weight(frame, i);
        float x = left + i * (width + gap);
        float icon = kit_s(screen, 34);

        draw_round_rect(screen, x, top, width, height, kit_s(screen, 8), 0, CINEMA_TILE, alpha);
        draw_round_rect(screen, x, top, width, height, kit_s(screen, 8), 0, 0x502c0c,
                        (int)(alpha * weight));
        kit_logo(screen, entry, &cinema.letter, x + (width - icon) / 2, top + (height - icon) / 2,
                 icon, 0xffe3a8, (int)(alpha * (0.45f + 0.55f * weight)));
    }

    outline = left + frame->position * (width + gap);
    draw_round_rect(screen, outline - 3, top - 3, width + 6, height + 6, kit_s(screen, 10), 3,
                    CINEMA_GOLD, alpha);
}

static void cinema_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &cinema.hint, CINEMA_WARM, 0 };
    float hints_x = kit_x(screen, 80);
    float strip_left = hints_x + widest_hints(context, frame, &hints) + kit_s(screen, 32);
    char clock[16];

    kit_draw_backdrop(screen, &cinema.backdrop);

    draw_feature(context, frame, fade.alpha);
    draw_filmstrip(context, frame, strip_left, fade.alpha);

    kit_hints(screen, &hints, hints_x,
              (int)(strip_top(screen) + kit_s(screen, 62) + cinema.hint.ascent / 2),
              &context->entries[frame->selected], frame, 0, fade.alpha * 170 / 256);

    str_copy(clock, frame->clock_time, sizeof(clock));
    str_append(clock, " ", sizeof(clock));
    str_append(clock, frame->clock_period, sizeof(clock));
    kit_text_right(screen, &cinema.clock, screen->width - kit_x(screen, 48),
                   (int)(kit_y(screen, 34) + cinema.clock.ascent), clock, CINEMA_LIGHT, fade.alpha);

    kit_splash(context, frame, &cinema.splash, &cinema.detail, &cinema.backdrop, CINEMA_LIGHT,
               CINEMA_GOLD);
    kit_intro_blackout(screen, frame);
}

static const EditColors cinema_edit = { 0x1a0f07, 0x0e0703, 0xf5b301, 0xfff4dc, 0xc9b48c, EDIT_FRAME_CARD };

const Theme cinema_theme = {
    .name = "cinema",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 0,
    .down_opens_list = 1,
    .edit = &cinema_edit,
    .load = cinema_load,
    .render = cinema_render
};