#include "aarambh/theme_kit.h"

#include "lib/math.h"
#include "lib/str.h"

#define HUD_LIME    0xc6ff00
#define HUD_PALE    0xe9ffb0
#define HUD_GRID    40.0f
#define HUD_NAMES   3

typedef struct {
    Face readout;
    Face title;
    Face list;
    Face names[HUD_NAMES];
    Face hint;
    Face clock;
    Face kernels;
    Face splash;
    Image backdrop;
    Image plain;
    Image glow;
} HudTheme;

static HudTheme hud;

static float target_x(const Image *screen)
{
    return screen->width / 2.0f;
}

static float target_y(const Image *screen)
{
    return screen->height * 0.47f;
}

static void paint_grid(Image *backdrop, const Image *screen)
{
    float step = kit_s(screen, HUD_GRID);
    float position;
    int y;

    for (position = 0; position < backdrop->width; position += step) {
        draw_rect(backdrop, (int)position, 0, 1, backdrop->height, HUD_LIME, 14);
    }

    for (position = 0; position < backdrop->height; position += step) {
        draw_rect(backdrop, 0, (int)position, backdrop->width, 1, HUD_LIME, 14);
    }

    for (y = 0; y < backdrop->height; y += 3) {
        draw_rect(backdrop, 0, y, backdrop->width, 1, COLOR_BLACK, 45);
    }
}

static void paint_bracket(Image *backdrop, const Image *screen, float half)
{
    float cx = target_x(screen);
    float cy = target_y(screen);
    float length = kit_s(screen, 28);
    float thickness = kit_s(screen, 3);
    int corner;

    for (corner = 0; corner < 4; corner++) {
        float dx = 1.0f;
        float dy = 1.0f;
        float x;
        float y;

        if (corner == 1 || corner == 3) {
            dx = -1.0f;
        }
        if (corner >= 2) {
            dy = -1.0f;
        }

        x = cx - dx * half;
        y = cy - dy * half;
        draw_line(backdrop, x, y, x + dx * length, y, thickness, HUD_LIME, 256);
        draw_line(backdrop, x, y, x, y + dy * length, thickness, HUD_LIME, 256);
    }
}

static void paint_target(Image *backdrop, const Image *screen)
{
    float cx = target_x(screen);
    float cy = target_y(screen);

    paint_bracket(backdrop, screen, kit_s(screen, 175));
    draw_ring(backdrop, cx, cy, kit_s(screen, 150), 1, HUD_LIME, 110);
    draw_line(backdrop, cx - kit_s(screen, 270), cy, cx - kit_s(screen, 185), cy, 1, HUD_LIME, 110);
    draw_line(backdrop, cx + kit_s(screen, 185), cy, cx + kit_s(screen, 270), cy, 1, HUD_LIME, 110);
    draw_text(backdrop, &hud.readout, kit_x(screen, 70),
              (int)(kit_y(screen, 96) + hud.readout.ascent),
              "// AARAMBH BOOT SYSTEM", HUD_LIME, 110);
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &hud.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x060a04, 0x0b1306)) {
        return;
    }

    kit_glow(backdrop, &hud.glow, target_x(screen), target_y(screen), kit_s(screen, 1000),
             kit_s(screen, 1000), 0x4d6b00, 90);
    paint_grid(backdrop, screen);

    if (image_alloc(&hud.plain, screen->width, screen->height)) {
        image_copy(&hud.plain, backdrop);
    }

    paint_target(backdrop, screen);
}

static void hud_load(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&hud.readout, context->font_mono, kit_s(screen, 16));
    face_init(&hud.list, context->font_mono, kit_s(screen, 18));
    face_init(&hud.names[0], context->font_mono, kit_s(screen, 52));
    face_init(&hud.names[1], context->font_mono, kit_s(screen, 38));
    face_init(&hud.names[2], context->font_mono, kit_s(screen, 28));
    face_init(&hud.hint, context->font_mono, kit_s(screen, 15));
    face_init(&hud.clock, context->font_mono, kit_s(screen, 28));
    face_init(&hud.kernels, context->font_mono, kit_s(screen, 18));
    face_init(&hud.splash, context->font_mono, kit_s(screen, 44));
    face_init(&hud.title, context->font_mono, kit_s(screen, 40));

    kit_glow_sprite(&hud.glow);
    build_backdrop(screen);
}

static void readout_line(Image *screen, int row, const char *label, const char *value, int alpha)
{
    char line[96];

    str_copy(line, label, sizeof(line));
    str_append(line, value, sizeof(line));
    draw_text(screen, &hud.readout, kit_x(screen, 70),
              (int)(kit_y(screen, 138) + row * kit_s(screen, 30) + hud.readout.ascent), line,
              HUD_LIME, alpha);
}

static void draw_readout(const ThemeContext *context, const ThemeFrame *frame, int index,
                         int alpha)
{
    const Entry *entry = &context->entries[index];
    const char *kernel = entry_launch_detail(entry, frame->launch_kernel);
    char name[64];
    char status[64];

    str_to_upper(name, entry->title, sizeof(name));
    str_copy(status, "MANUAL", sizeof(status));
    if (frame->status[0] != '\0') {
        str_to_upper(status, frame->status, sizeof(status));
    }
    if (kernel[0] == '\0') {
        kernel = "-";
    }

    readout_line(context->screen, 0, "TARGET     ", name, alpha);
    readout_line(context->screen, 1, "KERNEL     ", kernel, alpha);
    if (frame->launching) {
        readout_line(context->screen, 2, "STATUS     ", "ENGAGED", alpha);
    } else {
        readout_line(context->screen, 2, "STATUS     ", "READY", alpha);
    }
    readout_line(context->screen, 3, "AUTOBOOT   ", status, alpha);
}

static void draw_targets(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    int i;

    for (i = 0; i < context->entry_count; i++) {
        float weight = kit_weight(frame, i);
        char name[72];

        str_copy(name, "  ", sizeof(name));
        if (weight > 0.5f) {
            str_copy(name, "> ", sizeof(name));
        }
        str_to_upper(name + 2, context->entries[i].title, (int)sizeof(name) - 2);

        kit_text_right(screen, &hud.list, screen->width - kit_x(screen, 70),
                       (int)(kit_y(screen, 138) + i * kit_s(screen, 42) + hud.list.ascent), name,
                       HUD_LIME, (int)(alpha * (0.43f + 0.57f * weight)));
    }
}

static void draw_lock(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    float cx = target_x(screen);
    float cy = target_y(screen);
    float spin = frame->now / 900.0f;
    int shown = (int)(alpha * visibility);
    int content = (int)(shown * (1.0f - frame->list));
    Face *name_face;
    char name[64];

    draw_arc(screen, cx, cy, kit_s(screen, 132), kit_s(screen, 3), spin + 3.49f, spin + 5.76f,
             HUD_LIME, alpha);
    draw_arc(screen, cx, cy, kit_s(screen, 132), kit_s(screen, 3), spin + 0.35f, spin + 1.40f,
             HUD_LIME, alpha * 110 / 256);

    kit_glow(screen, &hud.glow, cx, cy, kit_s(screen, 260), kit_s(screen, 260), HUD_LIME,
             content * 60 / 256);
    kit_logo(screen, entry, &hud.title, cx - kit_s(screen, 75), cy - kit_s(screen, 75),
             kit_s(screen, 150), HUD_PALE, content);

    str_to_upper(name, entry->title, sizeof(name));
    name_face = kit_fit(hud.names, HUD_NAMES, name, kit_x(screen, 700));
    kit_text_center(screen, name_face, cx, (int)(cy + kit_s(screen, 200) + hud.names[0].ascent),
                    name, HUD_PALE, shown);

    draw_readout(context, frame, index, alpha);

    kit_kernel_list(screen, entry, frame, &hud.readout, &hud.kernels, cx - kit_s(screen, 240),
                    cy - kit_s(screen, 120), kit_s(screen, 480), HUD_PALE, HUD_LIME, shown);
}

static void hud_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &hud.hint, HUD_LIME, 0 };
    char clock[24];

    kit_draw_backdrop(screen, &hud.backdrop);

    draw_targets(context, frame, fade.alpha);
    draw_lock(context, frame, fade.alpha);

    kit_hints_center(screen, &hints, (int)(screen->height - kit_s(screen, 46)),
                     &context->entries[frame->selected], frame, 1, fade.alpha * 150 / 256);

    str_copy(clock, frame->clock_time, sizeof(clock));
    str_append(clock, " ", sizeof(clock));
    str_append(clock, frame->clock_period, sizeof(clock));
    kit_text_right(screen, &hud.clock, screen->width - kit_x(screen, 70),
                   (int)(screen->height - kit_y(screen, 104) + hud.clock.ascent), clock, HUD_LIME,
                   fade.alpha);

    kit_splash(context, frame, &hud.splash, &hud.readout, &hud.plain, HUD_PALE, HUD_LIME);
    kit_intro_blackout(screen, frame);
}

static const EditColors hud_edit = { 0x070c04, 0x030602, 0xc6ff00, 0xe9ffb0, 0x8fae3a, EDIT_FRAME_BRACKETS };

const Theme hud_theme = {
    .name = "hud",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 1,
    .down_opens_list = 0,
    .edit = &hud_edit,
    .load = hud_load,
    .render = hud_render
};