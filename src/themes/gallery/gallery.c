#include "aarambh/theme_kit.h"

#include "lib/str.h"

/*
 * Gallery: every entry is a card made from its wallpaper, laid out like a
 * cover flow. The selected card faces the viewer, the others turn away to
 * the sides, and all of them reflect on a dark floor.
 */

#define GALLERY_TEXT    0xeef0f4
#define GALLERY_MUTED   0x9ea3ad
#define GALLERY_DOT     0x4a4e57
#define GALLERY_TITLES  3

#define CARD_WIDTH      520.0f
#define CARD_HEIGHT     292.0f
#define CARD_TOP        92.0f
#define CARD_RADIUS     18.0f
#define LIST_LIFT       60.0f

#define SIDE_WIDTH      0.42f
#define SIDE_NEAR       0.80f
#define SIDE_FAR        0.60f
#define SIDE_OFFSET     395.0f
#define FAR_STEP        150.0f
#define FAR_SHRINK      0.08f
#define VISIBLE_RANGE   2.6f

#define REFLECTION_GAP    8.0f
#define REFLECTION_LENGTH 0.42f
#define REFLECTION_ALPHA  70

#define TITLE_Y         548.0f
#define DETAIL_Y        584.0f
#define BUTTON_Y        608.0f
#define BUTTON_HEIGHT   46.0f
#define DOTS_Y          690.0f

typedef struct {
    Face label;
    Face titles[GALLERY_TITLES];
    Face card_title;
    Face detail;
    Face button;
    Face hint;
    Face clock;
    Face period;
    Face list;
    Face letter;
    Face splash;
    Image backdrop;
    Image glow;
    Image cards[AARAMBH_MAX_ENTRIES];
} GalleryTheme;

static GalleryTheme gallery;

typedef struct {
    float x;
    float width;
    float left_height;
    float right_height;
    int alpha;
} CardPose;

static float lerp(float from, float to, float amount)
{
    return from + (to - from) * amount;
}

static void build_card(const ThemeContext *context, int index)
{
    const Image *screen = context->screen;
    const Entry *entry = &context->entries[index];
    Image *card = &gallery.cards[index];
    float logo = kit_s(screen, 46);
    float margin = kit_s(screen, 26);
    int fade_start;
    int y;

    if (!image_cover(card, &entry->background, (int)kit_s(screen, CARD_WIDTH),
                     (int)kit_s(screen, CARD_HEIGHT))) {
        return;
    }

    fade_start = card->height * 2 / 5;
    for (y = fade_start; y < card->height; y++) {
        draw_rect(card, 0, y, card->width, 1, COLOR_BLACK,
                  (y - fade_start) * 150 / (card->height - fade_start));
    }

    kit_logo(card, entry, &gallery.letter, margin, card->height - margin - logo, logo,
             COLOR_WHITE, 256);
    draw_text(card, &gallery.card_title, margin + logo + kit_s(screen, 16),
              (int)(card->height - margin - logo / 2 + gallery.card_title.ascent * 0.36f),
              entry->title, COLOR_WHITE, 256);

    image_make_opaque(card);
    image_round_corners(card, kit_s(screen, CARD_RADIUS));
}

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &gallery.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x202228, 0x070709)) {
        return;
    }

    kit_glow(backdrop, &gallery.glow, screen->width / 2.0f, kit_y(screen, 230),
             kit_s(screen, 1500), kit_s(screen, 720), 0x3a3d46, 200);
    draw_text(backdrop, &gallery.label, kit_x(screen, 64),
              (int)(kit_y(screen, 40) + gallery.label.ascent), "AARAMBH", GALLERY_MUTED, 256);
}

static void gallery_load(const ThemeContext *context)
{
    const Image *screen = context->screen;
    int i;

    face_init(&gallery.label, context->font_regular, kit_s(screen, 13));
    face_init(&gallery.titles[0], context->font_bold, kit_s(screen, 58));
    face_init(&gallery.titles[1], context->font_bold, kit_s(screen, 46));
    face_init(&gallery.titles[2], context->font_bold, kit_s(screen, 36));
    face_init(&gallery.card_title, context->font_bold, kit_s(screen, 30));
    face_init(&gallery.detail, context->font_regular, kit_s(screen, 17));
    face_init(&gallery.button, context->font_bold, kit_s(screen, 17));
    face_init(&gallery.hint, context->font_regular, kit_s(screen, 14));
    face_init(&gallery.clock, context->font_bold, kit_s(screen, 34));
    face_init(&gallery.period, context->font_bold, kit_s(screen, 15));
    face_init(&gallery.list, context->font_regular, kit_s(screen, 19));
    face_init(&gallery.letter, context->font_bold, kit_s(screen, 34));
    face_init(&gallery.splash, context->font_bold, kit_s(screen, 50));

    kit_glow_sprite(&gallery.glow);
    build_backdrop(screen);

    for (i = 0; i < context->entry_count; i++) {
        build_card(context, i);
    }
}

/* Where card `index` sits for the current, animated selection. */
static CardPose card_pose(const Image *screen, const ThemeFrame *frame, int index, int alpha)
{
    float offset = index - frame->position;
    float distance = absf(offset);
    float turn = clampf(distance, 0.0f, 1.0f);
    float beyond = clampf(distance - 1.0f, 0.0f, VISIBLE_RANGE);
    float shrink = 1.0f - FAR_SHRINK * beyond;
    float near = kit_s(screen, CARD_HEIGHT) * lerp(1.0f, SIDE_NEAR, turn) * shrink;
    float far = kit_s(screen, CARD_HEIGHT) * lerp(1.0f, SIDE_FAR, turn) * shrink;
    float shift = kit_s(screen, lerp(0.0f, SIDE_OFFSET, turn) + beyond * FAR_STEP);
    float fade = 1.0f - clampf(distance - 2.0f, 0.0f, 1.0f) / (VISIBLE_RANGE - 2.0f);
    CardPose pose;

    pose.width = kit_s(screen, CARD_WIDTH) * lerp(1.0f, SIDE_WIDTH, turn) * shrink;
    pose.left_height = near;
    pose.right_height = far;

    if (offset < 0) {
        shift = -shift;
        pose.left_height = far;
        pose.right_height = near;
    }

    pose.x = screen->width / 2.0f + shift - pose.width / 2;
    pose.alpha = (int)(alpha * clampf(fade, 0.0f, 1.0f) * (1.0f - 0.18f * turn));
    return pose;
}

static void draw_cards(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float center_y = kit_y(screen, CARD_TOP) + kit_s(screen, CARD_HEIGHT) / 2 -
                     frame->list * kit_s(screen, LIST_LIFT);
    int order[AARAMBH_MAX_ENTRIES];
    int count = context->entry_count;
    int i;
    int j;

    for (i = 0; i < count; i++) {
        order[i] = i;
    }

    for (i = 0; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if (absf(order[j] - frame->position) > absf(order[i] - frame->position)) {
                int swap = order[i];

                order[i] = order[j];
                order[j] = swap;
            }
        }
    }

    for (i = 0; i < count; i++) {
        int index = order[i];
        CardPose pose = card_pose(screen, frame, index, alpha);
        const Image *card = &gallery.cards[index];

        if (pose.alpha <= 0) {
            continue;
        }

        draw_image_reflection(screen, card, pose.x, center_y, pose.width, pose.left_height,
                              pose.right_height, kit_s(screen, REFLECTION_GAP), REFLECTION_LENGTH,
                              (int)(pose.alpha * REFLECTION_ALPHA / 256 * (1.0f - frame->list)));
        draw_image_perspective(screen, card, pose.x, center_y, pose.width, pose.left_height,
                               pose.right_height, pose.alpha);

        if (index == frame->selected) {
            draw_round_rect(screen, pose.x, center_y - pose.left_height / 2, pose.width,
                            pose.left_height, kit_s(screen, CARD_RADIUS), 1.2f, COLOR_WHITE,
                            (int)(pose.alpha * 50 / 256 *
                                  clampf((kit_weight(frame, index) - 0.8f) * 5.0f, 0.0f, 1.0f)));
        }
    }
}

static void draw_buttons(Image *screen, const Entry *entry, const ThemeFrame *frame, int alpha)
{
    UiButton primary;
    UiButton advanced;
    float gap = kit_s(screen, 14);
    float width;

    primary.x = 0;
    primary.y = kit_y(screen, BUTTON_Y);
    primary.height = kit_s(screen, BUTTON_HEIGHT);
    primary.label = entry_action_label(entry);
    primary.play_icon = 1;
    primary.active = 1.0f - frame->focus;
    primary.idle_fill = 0;
    primary.border = 256;
    primary.fill_color = GALLERY_TEXT;
    primary.text_color = COLOR_INK;

    advanced = primary;
    advanced.label = "Advanced options";
    advanced.play_icon = 0;
    advanced.active = frame->focus;
    advanced.idle_fill = 30;
    advanced.border = 130;

    width = ui_button_width(screen, &gallery.button, &primary);
    if (entry_has_kernels(entry)) {
        width += gap + ui_button_width(screen, &gallery.button, &advanced);
    }

    primary.x = (screen->width - width) / 2;
    advanced.x = ui_button_draw(screen, &gallery.button, &primary, alpha) + gap;

    if (entry_has_kernels(entry)) {
        ui_button_draw(screen, &gallery.button, &advanced, alpha);
    }
}

static void draw_details(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    int content = (int)(alpha * (1.0f - frame->list));
    int shown = (int)(content * visibility);
    float rise = (1.0f - visibility) * kit_s(screen, 12);
    Face *title = kit_fit(gallery.titles, GALLERY_TITLES, entry->title, kit_x(screen, 900));
    char line[192];

    kit_text_center(screen, title, screen->width / 2.0f, (int)(kit_y(screen, TITLE_Y) + rise),
                    entry->title, GALLERY_TEXT, shown);

    kit_detail_line(line, sizeof(line), context, frame, index, 1);
    kit_text_center(screen, &gallery.detail, screen->width / 2.0f,
                    (int)(kit_y(screen, DETAIL_Y) + rise), line, GALLERY_MUTED, shown);

    draw_buttons(screen, &context->entries[frame->selected], frame, shown);

    kit_kernel_list(screen, &context->entries[frame->selected], frame, &gallery.label,
                    &gallery.list, screen->width / 2.0f - kit_s(screen, 270), kit_y(screen, 440),
                    kit_s(screen, 540), GALLERY_TEXT, UI_ENTRY_ACCENT, alpha);
}

static void draw_dots(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float size = kit_s(screen, 8);
    float wide = kit_s(screen, 26);
    float gap = kit_s(screen, 9);
    float total = context->entry_count * (size + gap) - gap + (wide - size);
    float x = (screen->width - total) / 2;
    float y = kit_y(screen, DOTS_Y);
    int i;

    for (i = 0; i < context->entry_count; i++) {
        float weight = kit_weight(frame, i);
        float width = lerp(size, wide, weight);

        draw_round_rect(screen, x, y, width, size, size / 2, 0, GALLERY_DOT, alpha);
        draw_round_rect(screen, x, y, width, size, size / 2, 0, GALLERY_TEXT,
                        (int)(alpha * weight));
        x += width + gap;
    }
}

static void gallery_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &gallery.hint, GALLERY_MUTED, 0 };
    UiClock clock;

    kit_draw_backdrop(screen, &gallery.backdrop);
    draw_cards(context, frame, fade.alpha);
    draw_details(context, frame, fade.alpha);
    draw_dots(context, frame, (int)(fade.alpha * (1.0f - frame->list)));

    kit_hints_center(screen, &hints, (int)(screen->height - kit_s(screen, 36)),
                     &context->entries[frame->selected], frame, 0, fade.alpha * 200 / 256);

    clock.time_face = &gallery.clock;
    clock.period_face = &gallery.period;
    clock.right = screen->width - kit_x(screen, 64);
    clock.baseline = (int)(kit_y(screen, 30) + gallery.clock.ascent);
    clock.gap = kit_s(screen, 6);
    clock.time_color = GALLERY_TEXT;
    clock.period_color = GALLERY_MUTED;
    clock.shadow = 0;
    ui_clock_draw(screen, &clock, frame, fade.alpha);

    kit_splash(context, frame, &gallery.splash, &gallery.detail, NULL, GALLERY_TEXT,
               GALLERY_MUTED);
    kit_intro_blackout(screen, frame);
}

static const EditColors gallery_edit = { 0x16171b, 0x0c0c0e, UI_ENTRY_ACCENT, GALLERY_TEXT,
                                         GALLERY_MUTED, EDIT_FRAME_CARD };

const Theme gallery_theme = {
    .name = "gallery",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 0,
    .down_opens_list = 0,
    .edit = &gallery_edit,
    .load = gallery_load,
    .render = gallery_render
};