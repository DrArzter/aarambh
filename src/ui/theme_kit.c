#include "aarambh/theme_kit.h"

#include "lib/str.h"

KitFade kit_fade(const ThemeFrame *frame)
{
    KitFade fade;

    fade.intro = ease_out_cubic(frame->intro_elapsed / AARAMBH_INTRO_MS);
    fade.launch = 0.0f;

    if (frame->launching) {
        fade.launch = ease_in_out_cubic(frame->launch_elapsed / AARAMBH_LAUNCH_MS);
    }

    fade.alpha = (int)(fade.intro * (1.0f - fade.launch) * 256.0f);
    return fade;
}

void kit_intro_blackout(Image *screen, const ThemeFrame *frame)
{
    float progress = ease_out_cubic(frame->intro_elapsed / (AARAMBH_INTRO_MS * 0.55f));

    if (progress < 1.0f) {
        image_darken(screen, (int)((1.0f - progress) * 256.0f));
    }
}

float kit_weight(const ThemeFrame *frame, int index)
{
    return clampf(1.0f - absf(index - frame->position), 0.0f, 1.0f);
}

int kit_backdrop(Image *backdrop, const Image *screen, UINT32 top, UINT32 bottom)
{
    if (!image_alloc(backdrop, screen->width, screen->height)) {
        return 0;
    }

    image_gradient(backdrop, top, bottom);
    return 1;
}

#define GLOW_SPRITE_SIZE 128

int kit_glow_sprite(Image *sprite)
{
    return image_glow_sprite(sprite, GLOW_SPRITE_SIZE, COLOR_WHITE);
}

void kit_glow(Image *destination, const Image *sprite, float center_x, float center_y,
              float width, float height, UINT32 color, int alpha)
{
    if (sprite->pixels == NULL) {
        return;
    }

    draw_image_tinted(destination, sprite, (int)(center_x - width / 2),
                      (int)(center_y - height / 2), (int)width, (int)height, color, alpha);
}

void kit_draw_backdrop(Image *screen, const Image *backdrop)
{
    if (backdrop->pixels != NULL) {
        image_copy(screen, backdrop);
    } else {
        image_clear(screen);
    }
}

int kit_nearest(const ThemeFrame *frame, int entry_count, float *visibility)
{
    int index = (int)(frame->position + 0.5f);

    if (index < 0) {
        index = 0;
    }
    if (index >= entry_count) {
        index = entry_count - 1;
    }

    *visibility = clampf(1.0f - 2.0f * absf(frame->position - index), 0.0f, 1.0f);
    return index;
}

Face *kit_fit(Face *faces, int count, const char *text, float max_width)
{
    int i;

    for (i = 0; i < count - 1; i++) {
        if (text_width(&faces[i], text) <= max_width) {
            return &faces[i];
        }
    }

    return &faces[count - 1];
}

void kit_detail_line(char *output, int capacity, const ThemeContext *context,
                     const ThemeFrame *frame, int index, int with_status)
{
    const Entry *entry = &context->entries[index];

    entry_subtitle(entry, index == context->default_index, output, capacity);

    if (with_status && frame->status[0] != '\0' && index == frame->selected) {
        str_append(output, "  -  ", capacity);
        str_append(output, frame->status, capacity);
    }
}

void kit_stars(Image *destination, int count, UINT32 seed, UINT32 color, int max_alpha,
               float max_y)
{
    UINT32 state = seed;
    int i;

    for (i = 0; i < count; i++) {
        int x = (int)(image_random(&state) % (UINT32)destination->width);
        int y = (int)(image_random(&state) % (UINT32)max_y);
        int size = 1 + (int)(image_random(&state) % 3) / 2;
        int alpha = 40 + (int)(image_random(&state) % (UINT32)(max_alpha - 39));

        draw_rect(destination, x, y, size, size, color, alpha);
    }
}

void kit_logo(Image *screen, const Entry *entry, Face *letter_face, float x, float y,
              float size, UINT32 color, int alpha)
{
    char letter[2];

    if (entry_has_logo(entry)) {
        draw_image_tinted(screen, &entry->logo, (int)x, (int)y, (int)size, (int)size,
                          color, alpha);
        return;
    }

    letter[0] = entry->title[0];
    letter[1] = '\0';
    kit_text_center(screen, letter_face, x + size / 2,
                    (int)(y + size / 2 + letter_face->ascent * 0.38f), letter, color, alpha);
}

void kit_text_center(Image *screen, Face *face, float center_x, int baseline,
                     const char *text, UINT32 color, int alpha)
{
    draw_text(screen, face, center_x - text_width(face, text) / 2, baseline, text, color, alpha);
}

void kit_text_right(Image *screen, Face *face, float right, int baseline,
                    const char *text, UINT32 color, int alpha)
{
    draw_text(screen, face, right - text_width(face, text), baseline, text, color, alpha);
}

typedef struct {
    int kind;
    const char *label;
    UiArrowAxis axis;
} HintItem;

enum {
    HINT_KEY,
    HINT_ARROWS,
    HINT_LABEL
};

static int build_hints(HintItem *items, const Entry *entry, const ThemeFrame *frame, int vertical)
{
    int count = 0;
    UiArrowAxis move = UI_ARROWS_HORIZONTAL;
    UiArrowAxis options = UI_ARROWS_VERTICAL;

    if (vertical) {
        move = UI_ARROWS_VERTICAL;
        options = UI_ARROWS_HORIZONTAL;
    }

    if (frame->list > 0.5f) {
        items[count++] = (HintItem){ HINT_ARROWS, NULL, UI_ARROWS_VERTICAL };
        items[count++] = (HintItem){ HINT_LABEL, "Choose", UI_ARROWS_VERTICAL };
        items[count++] = (HintItem){ HINT_KEY, "Enter", UI_ARROWS_VERTICAL };
        items[count++] = (HintItem){ HINT_LABEL, "Boot", UI_ARROWS_VERTICAL };
        items[count++] = (HintItem){ HINT_KEY, "Esc", UI_ARROWS_VERTICAL };
        items[count++] = (HintItem){ HINT_LABEL, "Back", UI_ARROWS_VERTICAL };
        if (frame->can_edit) {
            items[count++] = (HintItem){ HINT_KEY, "E", UI_ARROWS_VERTICAL };
            items[count++] = (HintItem){ HINT_LABEL, "Edit", UI_ARROWS_VERTICAL };
        }
        return count;
    }

    items[count++] = (HintItem){ HINT_KEY, "Enter", move };
    items[count++] = (HintItem){ HINT_LABEL, entry_action_label(entry), move };
    items[count++] = (HintItem){ HINT_ARROWS, NULL, move };
    items[count++] = (HintItem){ HINT_LABEL, "Switch", move };

    if (entry_has_kernels(entry)) {
        items[count++] = (HintItem){ HINT_ARROWS, NULL, options };
        items[count++] = (HintItem){ HINT_LABEL, "Options", options };
    }

    if (frame->can_edit) {
        items[count++] = (HintItem){ HINT_KEY, "E", move };
        items[count++] = (HintItem){ HINT_LABEL, "Edit", move };
    }

    return count;
}

static float place_hints(Image *screen, const UiHintStyle *style, float x, int baseline,
                         const Entry *entry, const ThemeFrame *frame, int vertical, int alpha)
{
    HintItem items[10];
    int count = build_hints(items, entry, frame, vertical);
    float start = x;
    int i;

    for (i = 0; i < count; i++) {
        const HintItem *item = &items[i];
        int size = style->face->ascent;

        if (item->kind == HINT_KEY) {
            if (screen != NULL) {
                x = ui_key_cap(screen, style, x, baseline, item->label, alpha);
            } else {
                x += text_width(style->face, item->label) + 28.0f;
            }
        } else if (item->kind == HINT_ARROWS) {
            if (screen != NULL) {
                x = ui_arrows(screen, style, x, baseline, item->axis, alpha);
            } else if (item->axis == UI_ARROWS_VERTICAL) {
                x += (size * 6 / 10) * 4 + 6 + 14;
            } else {
                x += size * 2 + 14;
            }
        } else {
            if (screen != NULL) {
                x = ui_hint(screen, style, x, baseline, item->label, alpha);
            } else {
                x += text_width(style->face, item->label) + size * 2.0f;
            }
        }
    }

    return x - start;
}

float kit_hints_width(const UiHintStyle *style, const Entry *entry, const ThemeFrame *frame,
                      int vertical)
{
    return place_hints(NULL, style, 0, 0, entry, frame, vertical, 0);
}

void kit_hints(Image *screen, const UiHintStyle *style, float x, int baseline,
               const Entry *entry, const ThemeFrame *frame, int vertical, int alpha)
{
    place_hints(screen, style, x, baseline, entry, frame, vertical, alpha);
}

void kit_hints_center(Image *screen, const UiHintStyle *style, int baseline,
                      const Entry *entry, const ThemeFrame *frame, int vertical, int alpha)
{
    float width = kit_hints_width(style, entry, frame, vertical);

    kit_hints(screen, style, (screen->width - width) / 2, baseline, entry, frame, vertical, alpha);
}

void kit_kernel_list(Image *screen, const Entry *entry, const ThemeFrame *frame,
                     Face *heading_face, Face *row_face, float x, float top, float width,
                     UINT32 text_color, UINT32 accent, int alpha)
{
    UiOptionList list;

    if (frame->list <= 0.0f || !entry_has_kernels(entry)) {
        return;
    }

    list.heading_face = heading_face;
    list.row_face = row_face;
    list.heading = "ADVANCED OPTIONS";
    list.x = x;
    list.top = top + (1.0f - frame->list) * kit_s(screen, 40);
    list.width = width;
    list.row_height = kit_s(screen, 45);
    list.rows = KIT_LIST_ROWS;
    list.heading_color = text_color;
    list.text_color = text_color;
    list.highlight_color = text_color;
    list.accent_color = accent;
    list.shadow = 0;

    ui_option_list_draw(screen, &list, entry, frame, (int)(alpha * frame->list));
}

void kit_splash(const ThemeContext *context, const ThemeFrame *frame, Face *title_face,
                Face *detail_face, const Image *backdrop, UINT32 title_color,
                UINT32 detail_color)
{
    UiSplash splash;

    if (!frame->launching) {
        return;
    }

    splash.title_face = title_face;
    splash.detail_face = detail_face;
    splash.backdrop = backdrop;
    splash.title_color = title_color;
    splash.detail_color = detail_color;
    splash.dot_color = title_color;
    splash.darken = 40;
    splash.shadow = 0;

    if (((title_color >> 16) & 0xff) + ((title_color >> 8) & 0xff) + (title_color & 0xff) < 384) {
        splash.darken = 0;
    }

    ui_launch_splash_draw(context, frame, &splash);
}