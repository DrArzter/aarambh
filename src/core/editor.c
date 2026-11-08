#include "core/editor.h"

#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/timing.h"
#include "aarambh/widgets.h"
#include "boot/launcher.h"
#include "core/navigation.h"
#include "lib/str.h"

#define EDIT_CTRL_R        0x12
#define EDIT_MAX_LINES     4
#define EDIT_DIM           140
#define EDIT_CURSOR_BLINK  530.0f
#define EDIT_NOTE          "Only this boot uses the changes. Nothing is saved."

typedef struct {
    Face heading;
    Face info;
    Face text;
    Face note;
    Face mono_heading;
    Face mono_info;
    Face mono_note;
} EditorFaces;

typedef struct {
    Image *screen;
    const EditColors *colors;
    UINT32 accent;
    float scale;
    float x;
    float y;
    float width;
    float height;
    float field_x;
    float field_y;
    float field_width;
    float field_height;
    int columns;
    int first_line;
    int lines;
    int alpha;
} EditorLayout;

static const EditColors default_colors = {
    0x1b1a20, 0x111014, 0xffffff, 0xffffff, 0xa8a6b0, EDIT_FRAME_CARD
};

static EditorFaces faces;

void editor_load(const ThemeContext *context)
{
    float scale = context->screen->height / 800.0f;

    face_init(&faces.heading, context->font_bold, 15 * scale);
    face_init(&faces.info, context->font_regular, 16 * scale);
    face_init(&faces.text, context->font_mono, 19 * scale);
    face_init(&faces.note, context->font_regular, 15 * scale);
    face_init(&faces.mono_heading, context->font_mono, 15 * scale);
    face_init(&faces.mono_info, context->font_mono, 16 * scale);
    face_init(&faces.mono_note, context->font_mono, 14 * scale);
}

static int editor_kernel(const AppState *app)
{
    if (app->list_open) {
        return app->list_selected;
    }

    return -1;
}

static const Kernel *edited_kernel(const AppState *app, int kernel)
{
    const Entry *entry = &app->entries.items[app->selected];

    if (kernel < 0) {
        kernel = 0;
    }

    return &entry->kernels[kernel];
}

int editor_can_open(const AppState *app)
{
    const Entry *entry = &app->entries.items[app->selected];

    return app->config.allow_edit && !app->launching && !app->edit_open
        && launcher_boots_direct(entry, editor_kernel(app));
}

static void load_text(AppState *app)
{
    str_copy(app->edit_text, edited_kernel(app, app->edit_kernel)->cmdline, APP_EDIT_MAX);
    app->edit_length = (int)str_length(app->edit_text);
    app->edit_cursor = app->edit_length;
}

void editor_open(AppState *app, float now)
{
    app->edit_kernel = editor_kernel(app);
    app->edit_open = 1;
    load_text(app);
    tween_to(&app->edit, 1.0f, now);
}

static void close_editor(AppState *app, float now)
{
    app->edit_open = 0;
    tween_to(&app->edit, 0.0f, now);
}

static void insert_char(AppState *app, char c)
{
    int i;

    if (app->edit_length >= APP_EDIT_MAX - 1) {
        return;
    }

    for (i = app->edit_length; i > app->edit_cursor; i--) {
        app->edit_text[i] = app->edit_text[i - 1];
    }

    app->edit_text[app->edit_cursor] = c;
    app->edit_length++;
    app->edit_cursor++;
    app->edit_text[app->edit_length] = '\0';
}

static void delete_at(AppState *app, int position)
{
    int i;

    if (position < 0 || position >= app->edit_length) {
        return;
    }

    for (i = position; i < app->edit_length; i++) {
        app->edit_text[i] = app->edit_text[i + 1];
    }

    app->edit_length--;
    if (app->edit_cursor > position) {
        app->edit_cursor--;
    }
}

void editor_handle_key(AppState *app, const EFI_INPUT_KEY *key, float now)
{
    CHAR16 c = key->UnicodeChar;

    if (key->ScanCode == SCAN_ESC) {
        close_editor(app, now);
    } else if (c == CHAR_CARRIAGE_RETURN) {
        app->edit_applied = 1;
        app->edit_open = 0;
        tween_init(&app->edit, 0.0f, AARAMBH_FOCUS_MS);
        nav_launch(app, app->edit_kernel, now);
    } else if (key->ScanCode == SCAN_LEFT && app->edit_cursor > 0) {
        app->edit_cursor--;
    } else if (key->ScanCode == SCAN_RIGHT && app->edit_cursor < app->edit_length) {
        app->edit_cursor++;
    } else if (key->ScanCode == SCAN_HOME) {
        app->edit_cursor = 0;
    } else if (key->ScanCode == SCAN_END) {
        app->edit_cursor = app->edit_length;
    } else if (key->ScanCode == SCAN_DELETE) {
        delete_at(app, app->edit_cursor);
    } else if (c == CHAR_BACKSPACE) {
        delete_at(app, app->edit_cursor - 1);
    } else if (c == EDIT_CTRL_R) {
        load_text(app);
    } else if (c >= 0x20 && c < 0x7f) {
        insert_char(app, (char)c);
    }
}

const char *editor_cmdline(const AppState *app)
{
    if (!app->edit_applied) {
        return NULL;
    }

    return app->edit_text;
}

static Face *heading_face(const EditorLayout *layout)
{
    return layout->colors->frame == EDIT_FRAME_BRACKETS ? &faces.mono_heading : &faces.heading;
}

static Face *info_face(const EditorLayout *layout)
{
    return layout->colors->frame == EDIT_FRAME_BRACKETS ? &faces.mono_info : &faces.info;
}

static Face *note_face(const EditorLayout *layout)
{
    return layout->colors->frame == EDIT_FRAME_BRACKETS ? &faces.mono_note : &faces.note;
}

static void plan_layout(EditorLayout *layout, const AppState *app, Image *screen, float shown)
{
    float scale = screen->height / 800.0f;
    float advance = text_width(&faces.text, "M");
    int needed;
    int cursor_line;

    layout->screen = screen;
    layout->colors = app->theme->edit != NULL ? app->theme->edit : &default_colors;
    layout->accent = layout->colors->accent;
    if (layout->accent == UI_ENTRY_ACCENT) {
        layout->accent = app->entries.items[app->selected].accent;
    }

    layout->scale = scale;
    layout->alpha = (int)(256 * shown);
    layout->width = 880 * scale;
    if (layout->width > screen->width - 80 * scale) {
        layout->width = screen->width - 80 * scale;
    }

    layout->field_width = layout->width - 72 * scale;
    layout->columns = (int)((layout->field_width - 44 * scale) / advance);
    if (layout->columns < 8) {
        layout->columns = 8;
    }

    needed = app->edit_length / layout->columns + 1;
    layout->lines = needed < 2 ? 2 : (needed > EDIT_MAX_LINES ? EDIT_MAX_LINES : needed);

    cursor_line = app->edit_cursor / layout->columns;
    layout->first_line = cursor_line - layout->lines + 1;
    if (layout->first_line < 0) {
        layout->first_line = 0;
    }

    layout->field_height = (46 + 36 * layout->lines) * scale;
    layout->height = (258 + 36 * layout->lines) * scale;
    layout->x = (screen->width - layout->width) / 2;
    layout->y = (screen->height - layout->height) / 2 + (1.0f - shown) * 24 * scale;
    layout->field_x = layout->x + 36 * scale;
    layout->field_y = layout->y + 86 * scale;
}

static void draw_brackets(const EditorLayout *layout)
{
    float s = layout->scale;
    int length = (int)(26 * s);
    int thick = (int)(3 * s) + 1;
    int left = (int)layout->x;
    int top = (int)layout->y;
    int right = (int)(layout->x + layout->width);
    int bottom = (int)(layout->y + layout->height);

    draw_rect(layout->screen, left, top, length, thick, layout->accent, layout->alpha);
    draw_rect(layout->screen, left, top, thick, length, layout->accent, layout->alpha);
    draw_rect(layout->screen, right - length, top, length, thick, layout->accent, layout->alpha);
    draw_rect(layout->screen, right - thick, top, thick, length, layout->accent, layout->alpha);
    draw_rect(layout->screen, left, bottom - thick, length, thick, layout->accent, layout->alpha);
    draw_rect(layout->screen, left, bottom - length, thick, length, layout->accent, layout->alpha);
    draw_rect(layout->screen, right - length, bottom - thick, length, thick, layout->accent,
              layout->alpha);
    draw_rect(layout->screen, right - thick, bottom - length, thick, length, layout->accent,
              layout->alpha);
}

static void draw_frame_and_field(const EditorLayout *layout)
{
    const EditColors *colors = layout->colors;
    Image *screen = layout->screen;
    float s = layout->scale;
    float radius = colors->frame == EDIT_FRAME_CARD ? 18 * s : 0;
    int alpha = layout->alpha;

    draw_round_rect(screen, layout->x, layout->y, layout->width, layout->height, radius, 0,
                    colors->panel, alpha * 246 / 256);

    if (colors->frame == EDIT_FRAME_RULES) {
        int rule = (int)(3 * s) + 1;

        draw_rect(screen, (int)layout->x, (int)layout->y, (int)layout->width, rule, colors->text,
                  alpha);
        draw_rect(screen, (int)layout->x, (int)(layout->y + layout->height) - rule,
                  (int)layout->width, rule, colors->text, alpha);
        draw_rect(screen, (int)layout->field_x, (int)(layout->field_y + layout->field_height),
                  (int)layout->field_width, (int)(2 * s) + 1, layout->accent, alpha);
        return;
    }

    draw_round_rect(screen, layout->x, layout->y, layout->width, layout->height, radius,
                    1.5f * s, layout->accent, alpha * 115 / 256);
    draw_round_rect(screen, layout->field_x, layout->field_y, layout->field_width,
                    layout->field_height, radius > 0 ? 10 * s : 0, 0, colors->field, alpha);

    if (colors->frame == EDIT_FRAME_BRACKETS) {
        draw_brackets(layout);
    }
}

static void draw_heading(const EditorLayout *layout, const AppState *app)
{
    const Entry *entry = &app->entries.items[app->selected];
    Face *face = info_face(layout);
    char heading[32];
    char info[160];
    float baseline = layout->y + 50 * layout->scale;

    str_copy(heading, "EDIT BOOT OPTIONS", sizeof(heading));
    if (layout->colors->frame == EDIT_FRAME_BRACKETS) {
        str_copy(heading, "// EDIT BOOT OPTIONS", sizeof(heading));
    }

    str_copy(info, entry->title, sizeof(info));
    str_append(info, "  -  ", sizeof(info));
    str_append(info, edited_kernel(app, app->edit_kernel)->title, sizeof(info));

    draw_text(layout->screen, heading_face(layout), layout->field_x, (int)baseline, heading,
              layout->accent, layout->alpha);
    draw_text(layout->screen, face,
              layout->x + layout->width - 36 * layout->scale - text_width(face, info),
              (int)baseline, info, layout->colors->muted, layout->alpha);
}

static void draw_text_lines(const EditorLayout *layout, const AppState *app, float now)
{
    float s = layout->scale;
    float advance = text_width(&faces.text, "M");
    float text_x = layout->field_x + 22 * s;
    char line[APP_EDIT_MAX];
    int cursor_line = app->edit_cursor / layout->columns;
    int i;

    for (i = 0; i < layout->lines; i++) {
        int index = layout->first_line + i;
        int start = index * layout->columns;
        int baseline = (int)(layout->field_y + (44 + 36 * i) * s);
        int count = app->edit_length - start;

        if (count > layout->columns) {
            count = layout->columns;
        }

        if (count > 0) {
            mem_copy(line, app->edit_text + start, (mem_size_t)count);
            line[count] = '\0';
            draw_text(layout->screen, &faces.text, text_x, baseline, line, layout->colors->text,
                      layout->alpha);
        }

        if (index == cursor_line && (int)(now / EDIT_CURSOR_BLINK) % 2 == 0) {
            float x = text_x + (app->edit_cursor - start) * advance;

            draw_rect(layout->screen, (int)x, baseline - faces.text.ascent - (int)(2 * s),
                      (int)(3 * s) + 1, face_line_height(&faces.text), layout->accent,
                      layout->alpha);
        }
    }
}

static void draw_footer(const EditorLayout *layout)
{
    float s = layout->scale;
    UiHintStyle hints = { note_face(layout), layout->colors->muted, 0 };
    float x = layout->field_x;
    int note_baseline = (int)(layout->field_y + layout->field_height + 40 * s);
    int hint_baseline = (int)(layout->y + layout->height - 36 * s);

    draw_text(layout->screen, note_face(layout), x, note_baseline, EDIT_NOTE,
              layout->colors->muted, layout->alpha);

    x = ui_key_cap(layout->screen, &hints, x, hint_baseline, "Enter", layout->alpha);
    x = ui_hint(layout->screen, &hints, x, hint_baseline, "Boot", layout->alpha);
    x = ui_key_cap(layout->screen, &hints, x, hint_baseline, "Esc", layout->alpha);
    x = ui_hint(layout->screen, &hints, x, hint_baseline, "Cancel", layout->alpha);
    x = ui_key_cap(layout->screen, &hints, x, hint_baseline, "Ctrl+R", layout->alpha);
    x = ui_hint(layout->screen, &hints, x, hint_baseline, "Reset", layout->alpha);
    x = ui_arrows(layout->screen, &hints, x, hint_baseline, UI_ARROWS_HORIZONTAL, layout->alpha);
    ui_hint(layout->screen, &hints, x, hint_baseline, "Move", layout->alpha);
}

void editor_draw(AppState *app, float now)
{
    float shown = tween_value(&app->edit, now);
    EditorLayout layout;

    if (shown <= 0.0f) {
        return;
    }

    image_darken(&app->screen, (int)(EDIT_DIM * shown));
    plan_layout(&layout, app, &app->screen, shown);
    draw_frame_and_field(&layout);
    draw_heading(&layout, app);
    draw_text_lines(&layout, app, now);
    draw_footer(&layout);
}