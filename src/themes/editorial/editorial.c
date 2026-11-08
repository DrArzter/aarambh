#include "aarambh/theme_kit.h"

#include "lib/str.h"

#define EDITORIAL_NAMES 3
#define EDITORIAL_ROW   74.0f

typedef struct {
    UINT32 paper;
    UINT32 ink;
    UINT32 red;
} EditorialPalette;

static const EditorialPalette editorial_light = { 0xefe9dd, 0x141210, 0xe63b2e };
static const EditorialPalette editorial_dark = { 0x15130f, 0xebe3d2, 0xff5a45 };

typedef struct {
    const EditorialPalette *palette;
    Face header;
    Face number;
    Face names[EDITORIAL_NAMES];
    Face subtitle;
    Face note;
    Face index;
    Face row;
    Face row_selected;
    Face hint;
    Face list;
    Face splash;
    Image backdrop;
} EditorialTheme;

static EditorialTheme editorial;

static void build_backdrop(const Image *screen)
{
    Image *backdrop = &editorial.backdrop;
    float right = screen->width - kit_x(screen, 60);

    if (!kit_backdrop(backdrop, screen, editorial.palette->paper, editorial.palette->paper)) {
        return;
    }

    image_noise(backdrop, 6, 0xed170a1);
    draw_rect(backdrop, (int)kit_x(screen, 60), (int)kit_y(screen, 60),
              (int)(right - kit_x(screen, 60)), (int)kit_s(screen, 2), editorial.palette->ink, 256);
    draw_rect(backdrop, (int)kit_x(screen, 60), (int)(screen->height - kit_y(screen, 62)),
              (int)(right - kit_x(screen, 60)), (int)kit_s(screen, 2), editorial.palette->ink, 256);
    draw_rect(backdrop, (int)(screen->width - kit_x(screen, 400)), (int)kit_y(screen, 140), 1,
              (int)(screen->height - kit_y(screen, 260)), editorial.palette->ink, 80);
    draw_text(backdrop, &editorial.header, kit_x(screen, 60),
              (int)(kit_y(screen, 76) + editorial.header.ascent), "AARAMBH",
              editorial.palette->ink, 256);
}

static void editorial_load_faces(const ThemeContext *context)
{
    const Image *screen = context->screen;

    face_init(&editorial.header, context->font_bold, kit_s(screen, 14));
    face_init(&editorial.number, context->font_bold, kit_s(screen, 300));
    face_init(&editorial.names[0], context->font_bold, kit_s(screen, 140));
    face_init(&editorial.names[1], context->font_bold, kit_s(screen, 96));
    face_init(&editorial.names[2], context->font_bold, kit_s(screen, 64));
    face_init(&editorial.subtitle, context->font_regular, kit_s(screen, 22));
    face_init(&editorial.note, context->font_light, kit_s(screen, 20));
    face_init(&editorial.index, context->font_bold, kit_s(screen, 16));
    face_init(&editorial.row, context->font_regular, kit_s(screen, 26));
    face_init(&editorial.row_selected, context->font_bold, kit_s(screen, 26));
    face_init(&editorial.hint, context->font_regular, kit_s(screen, 14));
    face_init(&editorial.list, context->font_regular, kit_s(screen, 22));
    face_init(&editorial.splash, context->font_bold, kit_s(screen, 56));

    build_backdrop(screen);
}

static void editorial_load(const ThemeContext *context)
{
    editorial.palette = &editorial_light;
    editorial_load_faces(context);
}

static void editorial_dark_load(const ThemeContext *context)
{
    editorial.palette = &editorial_dark;
    editorial_load_faces(context);
}

static void two_digits(char *output, int value)
{
    output[0] = (char)('0' + (value / 10) % 10);
    output[1] = (char)('0' + value % 10);
    output[2] = '\0';
}

static void draw_masthead(Image *screen, const ThemeContext *context, const ThemeFrame *frame,
                          int alpha)
{
    char line[64];
    char number[3];

    str_copy(line, "ISSUE ", sizeof(line));
    two_digits(number, frame->selected + 1);
    str_append(line, number, sizeof(line));
    str_append(line, " / ", sizeof(line));
    two_digits(number, context->entry_count);
    str_append(line, number, sizeof(line));
    str_append(line, "   -   ", sizeof(line));
    str_append(line, frame->clock_time, sizeof(line));
    str_append(line, " ", sizeof(line));
    str_append(line, frame->clock_period, sizeof(line));

    kit_text_right(screen, &editorial.header, screen->width - kit_x(screen, 60),
                   (int)(kit_y(screen, 76) + editorial.header.ascent), line,
                   editorial.palette->ink, alpha);
}

static void draw_feature(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    int shown = (int)(alpha * visibility);
    int content = (int)(shown * (1.0f - frame->list));
    float rise = (1.0f - visibility) * kit_s(screen, 30) - frame->list * kit_s(screen, 180);
    float max_width = screen->width - kit_x(screen, 520);
    char number[3];
    char name[64];
    char line[128];
    Face *face;

    two_digits(number, index + 1);
    draw_text(screen, &editorial.number, kit_x(screen, 52),
              (int)(kit_y(screen, 90) + editorial.number.ascent * 0.9f + rise), number,
              editorial.palette->red, content);

    str_to_upper(name, entry->title, sizeof(name));
    face = kit_fit(editorial.names, EDITORIAL_NAMES, name, max_width);
    draw_text(screen, face, kit_x(screen, 58),
              (int)(kit_y(screen, 410) + editorial.names[0].ascent * 0.9f + rise),
              name, editorial.palette->ink, shown);

    draw_rect(screen, (int)kit_x(screen, 64), (int)(kit_y(screen, 580) + rise),
              (int)kit_s(screen, 120), (int)kit_s(screen, 8), editorial.palette->red, shown);

    kit_detail_line(line, sizeof(line), context, frame, index, 0);
    draw_text(screen, &editorial.subtitle, kit_x(screen, 64),
              (int)(kit_y(screen, 610) + editorial.subtitle.ascent), line,
              editorial.palette->ink, content);

    if (frame->status[0] != '\0') {
        str_copy(line, "Press Enter to begin.  ", sizeof(line));
        str_append(line, frame->status, sizeof(line));
        str_append(line, ".", sizeof(line));
    } else {
        str_copy(line, "Press Enter to begin.", sizeof(line));
    }
    draw_text(screen, &editorial.note, kit_x(screen, 64),
              (int)(kit_y(screen, 648) + editorial.note.ascent), line, editorial.palette->ink,
              content * 170 / 256);

    kit_kernel_list(screen, entry, frame, &editorial.hint, &editorial.list, kit_x(screen, 64),
                    kit_y(screen, 455), max_width - kit_x(screen, 60), editorial.palette->ink,
                    editorial.palette->red, shown);
}

static void draw_contents(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float column = screen->width - kit_x(screen, 370);
    float underline = kit_y(screen, 170) + frame->position * kit_s(screen, EDITORIAL_ROW);
    int i;

    for (i = 0; i < context->entry_count; i++) {
        float weight = kit_weight(frame, i);
        float top = kit_y(screen, 170) + i * kit_s(screen, EDITORIAL_ROW);
        int row_alpha = (int)(alpha * (0.47f + 0.53f * weight));
        Face *face = &editorial.row;
        char number[3];
        char name[64];

        if (weight > 0.5f) {
            face = &editorial.row_selected;
        }

        two_digits(number, i + 1);
        str_to_upper(name, context->entries[i].title, sizeof(name));

        draw_text(screen, &editorial.index, column,
                  (int)(top + kit_s(screen, 4) + editorial.index.ascent),
                  number, editorial.palette->ink, row_alpha);
        draw_text(screen, &editorial.index, column,
                  (int)(top + kit_s(screen, 4) + editorial.index.ascent),
                  number, editorial.palette->red, (int)(alpha * weight));
        draw_text(screen, face, column + kit_s(screen, 50), (int)(top + face->ascent), name,
                  editorial.palette->ink, row_alpha);
    }

    draw_rect(screen, (int)(column + kit_s(screen, 50)), (int)(underline + kit_s(screen, 40)),
              (int)kit_s(screen, 60), (int)kit_s(screen, 4), editorial.palette->red, alpha);
}

static void editorial_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &editorial.hint, editorial.palette->ink, 0 };
    const Entry *entry = &context->entries[frame->selected];
    float width = kit_hints_width(&hints, entry, frame, 1);

    kit_draw_backdrop(screen, &editorial.backdrop);

    draw_masthead(screen, context, frame, fade.alpha);
    draw_feature(context, frame, fade.alpha);
    draw_contents(context, frame, fade.alpha);

    kit_hints(screen, &hints, screen->width - kit_x(screen, 60) - width,
              (int)(screen->height - kit_y(screen, 90) + editorial.hint.ascent), entry, frame, 1,
              fade.alpha * 170 / 256);

    kit_splash(context, frame, &editorial.splash, &editorial.subtitle, &editorial.backdrop,
               editorial.palette->ink, editorial.palette->red);
    kit_intro_blackout(screen, frame);
}

static const EditColors editorial_edit = {
    0xefe9dd, 0xefe9dd, 0xe63b2e, 0x141210, 0x6e665a, EDIT_FRAME_RULES
};

static const EditColors editorial_dark_edit = {
    0x15130f, 0x15130f, 0xff5a45, 0xebe3d2, 0x9a9284, EDIT_FRAME_RULES
};

const Theme editorial_theme = {
    .name = "editorial",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 1,
    .down_opens_list = 0,
    .edit = &editorial_edit,
    .load = editorial_load,
    .render = editorial_render
};

const Theme editorial_dark_theme = {
    .name = "editorial-dark",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 1,
    .down_opens_list = 0,
    .edit = &editorial_dark_edit,
    .load = editorial_dark_load,
    .render = editorial_render
};