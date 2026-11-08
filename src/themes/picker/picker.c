#include "aarambh/theme_kit.h"

#define PICKER_MINT    0x3ddc97
#define PICKER_TEXT    0xeafff5
#define PICKER_ROW     0xeceef0
#define PICKER_SOFT    0xc8d2ce
#define PICKER_TOP     0x1a1c20
#define PICKER_BOTTOM  0x0d0e10
#define PICKER_TITLES  3
#define PICKER_SIZES   4
#define PICKER_SPACING 92.0f
#define PICKER_REACH   3.2f

typedef struct {
    Face titles[PICKER_TITLES];
    Face rows[PICKER_SIZES];
    Face detail;
    Face hint;
    Face clock;
    Face clock_period;
    Face list;
    Face splash;
    Image backdrop;
    Image glow;
} PickerTheme;

static PickerTheme picker;

static float wheel_left(const Image *screen)
{
    return kit_x(screen, 600);
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &picker.backdrop;

    if (!kit_backdrop(backdrop, screen, PICKER_TOP, PICKER_BOTTOM)) {
        return;
    }

    kit_glow(backdrop, &picker.glow, kit_x(screen, 900), screen->height / 2.0f,
             kit_s(screen, 1100), kit_s(screen, 420), PICKER_MINT, 40);
}

static void picker_load(const ThemeContext *context)
{
    const Image *screen = context->screen;
    int i;

    face_init(&picker.titles[0], context->font_bold, kit_s(screen, 82));
    face_init(&picker.titles[1], context->font_bold, kit_s(screen, 62));
    face_init(&picker.titles[2], context->font_bold, kit_s(screen, 44));

    for (i = 0; i < PICKER_SIZES; i++) {
        face_init(&picker.rows[i], context->font_bold, kit_s(screen, 46.0f * (1.0f - 0.17f * i)));
    }

    face_init(&picker.detail, context->font_regular, kit_s(screen, 16));
    face_init(&picker.hint, context->font_regular, kit_s(screen, 14));
    face_init(&picker.clock, context->font_bold, kit_s(screen, 40));
    face_init(&picker.clock_period, context->font_bold, kit_s(screen, 17));
    face_init(&picker.list, context->font_regular, kit_s(screen, 20));
    face_init(&picker.splash, context->font_bold, kit_s(screen, 52));

    kit_glow_sprite(&picker.glow);
    build_backdrop(screen);
}

static void draw_wheel(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float center = screen->height / 2.0f;
    float left = wheel_left(screen);
    float bar_left = left - kit_s(screen, 40);
    float bar_right = screen->width - kit_x(screen, 110);
    int wheel_alpha = (int)(alpha * (1.0f - frame->list));
    int i;

    draw_round_rect(screen, bar_left, center - kit_s(screen, 46), bar_right - bar_left,
                    kit_s(screen, 92), kit_s(screen, 22), 0, PICKER_MINT, wheel_alpha * 34 / 256);
    draw_round_rect(screen, bar_left, center - kit_s(screen, 46), bar_right - bar_left,
                    kit_s(screen, 92), kit_s(screen, 22), 2, PICKER_MINT, wheel_alpha * 150 / 256);

    for (i = 0; i < context->entry_count; i++) {
        const Entry *entry = &context->entries[i];
        float offset = i - frame->position;
        float distance = absf(offset);
        float scale = 1.0f - 0.17f * distance;
        float fade = 1.0f - 0.3f * distance;
        float y = center + offset * kit_s(screen, PICKER_SPACING);
        int size_index = (int)(distance + 0.5f);
        Face *face;
        float logo;
        int row_alpha;

        if (distance > PICKER_REACH || fade <= 0) {
            continue;
        }

        if (size_index >= PICKER_SIZES) {
            size_index = PICKER_SIZES - 1;
        }

        face = &picker.rows[size_index];
        logo = kit_s(screen, 46) * scale;
        row_alpha = (int)(wheel_alpha * fade);

        if (distance < 0.5f) {
            kit_logo(screen, entry, face, left, y - logo / 2, logo, PICKER_MINT, row_alpha);
            draw_text(screen, face, left + kit_s(screen, 80), (int)(y + face->ascent * 0.38f),
                      entry->title, PICKER_TEXT, row_alpha);
            kit_text_right(screen, &picker.detail, bar_right - kit_s(screen, 30),
                           (int)(y + picker.detail.ascent * 0.38f), entry_launch_detail(entry, -1),
                           PICKER_MINT, row_alpha * 220 / 256);
        } else {
            kit_logo(screen, entry, face, left, y - logo / 2, logo, PICKER_ROW, row_alpha);
            draw_text(screen, face, left + kit_s(screen, 80), (int)(y + face->ascent * 0.38f),
                      entry->title, PICKER_ROW, row_alpha);
        }
    }
}

static void draw_wheel_fades(Image *screen)
{
    int reach = (int)kit_s(screen, 260);
    int left = (int)kit_x(screen, 500);
    int i;

    for (i = 0; i < reach; i++) {
        int strength = (reach - i) * 240 / reach;

        draw_rect(screen, left, i, screen->width - left, 1, PICKER_TOP, strength);
        draw_rect(screen, left, screen->height - 1 - i, screen->width - left, 1, PICKER_BOTTOM,
                  strength);
    }
}

static void draw_summary(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    int shown = (int)(alpha * visibility);
    Face *title = kit_fit(picker.titles, PICKER_TITLES, entry->title, kit_x(screen, 440));
    const char *note = "Spin with Up / Down, Enter to start";

    if (frame->status[0] != '\0') {
        note = frame->status;
    }

    kit_logo(screen, entry, &picker.titles[0], kit_x(screen, 110), kit_y(screen, 230),
             kit_s(screen, 120), PICKER_MINT, shown);
    draw_text(screen, title, kit_x(screen, 104),
              (int)(kit_y(screen, 380) + picker.titles[0].ascent),
              entry->title, PICKER_TEXT, shown);
    draw_text(screen, &picker.detail, kit_x(screen, 110),
              (int)(kit_y(screen, 484) + picker.detail.ascent), note, PICKER_SOFT,
              alpha * 220 / 256);
    draw_rect(screen, (int)kit_x(screen, 110), (int)kit_y(screen, 530), (int)kit_s(screen, 80),
              (int)kit_s(screen, 4), PICKER_MINT, alpha);

    kit_kernel_list(screen, entry, frame, &picker.hint, &picker.list, wheel_left(screen),
                    screen->height / 2.0f - kit_s(screen, 110),
                    screen->width - wheel_left(screen) - kit_x(screen, 150), PICKER_TEXT,
                    PICKER_MINT, shown);
}

static void picker_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &picker.hint, PICKER_SOFT, 0 };
    UiClock clock;

    kit_draw_backdrop(screen, &picker.backdrop);

    draw_wheel(context, frame, fade.alpha);
    draw_wheel_fades(screen);
    draw_summary(context, frame, fade.alpha);

    kit_hints(screen, &hints, kit_x(screen, 110), (int)(screen->height - kit_s(screen, 40)),
              &context->entries[frame->selected], frame, 1, fade.alpha * 200 / 256);

    clock.time_face = &picker.clock;
    clock.period_face = &picker.clock_period;
    clock.right = screen->width - kit_x(screen, 48);
    clock.baseline = (int)(kit_y(screen, 28) + picker.clock.ascent);
    clock.gap = kit_s(screen, 6);
    clock.time_color = PICKER_TEXT;
    clock.period_color = PICKER_TEXT;
    clock.shadow = 0;
    ui_clock_draw(screen, &clock, frame, fade.alpha);

    kit_splash(context, frame, &picker.splash, &picker.detail, &picker.backdrop, PICKER_TEXT,
               PICKER_MINT);
    kit_intro_blackout(screen, frame);
}

static const EditColors picker_edit = { 0x1a1c20, 0x0d0e10, 0x3ddc97, 0xeafff5, 0x9fb0a9, EDIT_FRAME_CARD };

const Theme picker_wheel_theme = {
    .name = "picker-wheel",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 1,
    .down_opens_list = 0,
    .edit = &picker_edit,
    .load = picker_load,
    .render = picker_render
};