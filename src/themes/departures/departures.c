#include "aarambh/theme_kit.h"

#include "lib/str.h"

/*
 * Departures: an airport split-flap board. Every entry is a flight with its
 * kernel, status and gate; changing the selection flips the letters that
 * change, like a real board.
 */

#define DEP_SKY     0x4fb3ff
#define DEP_TEXT    0xf2f2ee
#define DEP_DIM     0xc9ccd1
#define DEP_MUTED   0x9aa0a8
#define DEP_LABEL   0x6f7680
#define DEP_CELL    0x202329
#define DEP_SPLIT   0x0b0c0f
#define DEP_RULE    0x262a31
#define DEP_DELAY   0xff8a65

#define NAME_CELLS      12
#define KERNEL_CELLS    9
#define STATUS_CELLS    9
#define GATE_CELLS      2
#define CLOCK_CELLS     8
#define COUNT_CELLS     2
#define TEXT_CELLS      16

#define LEFT            70.0f
#define RIGHT           1210.0f
#define LOGO_X          80.0f
#define NAME_X          130.0f
#define KERNEL_X        560.0f
#define STATUS_X        830.0f
#define GATE_X          1124.0f
#define BOARD_TOP       200.0f
#define BOARD_BOTTOM    630.0f
#define ROW_MAX         70.0f
#define CELL_GAP        3.0f
#define FOOTER_Y        682.0f

#define FLIP_MS         70.0f
#define INTRO_ROW_MS    90.0f
#define INTRO_FLIP_MS   650.0f

typedef struct {
    float width;
    float height;
    float cap;
    Face face;
} CellSize;

typedef struct {
    Face label;
    Face title;
    Face text;
    Face hint;
    Face list;
    Face splash;
    Face detail;
    CellSize big;
    CellSize small;
    CellSize clock;
    float row_height;
    float footer_cells_x;
    Image backdrop;
    Image plain;
    Image glow;
} DeparturesTheme;

static DeparturesTheme board;

static float row_top(const Image *screen, float index)
{
    return kit_y(screen, BOARD_TOP) + index * board.row_height;
}

static float cell_step(const CellSize *size)
{
    return size->width + size->height * CELL_GAP / 40.0f;
}

static void size_cells(CellSize *size, Font *font, float height)
{
    size->height = height;
    size->width = height * 0.74f;
    size->cap = height * 0.62f * 0.73f;
    face_init(&size->face, font, height * 0.62f);
}

static void draw_cell(Image *image, const CellSize *size, float x, float y, int alpha)
{
    draw_round_rect(image, x, y, size->width, size->height, size->height * 0.12f, 0, DEP_CELL,
                    alpha);
}

static void draw_split(Image *image, const CellSize *size, float x, float y, int alpha)
{
    int thickness = (int)(size->height / 40.0f) + 1;

    draw_rect(image, (int)x, (int)(y + size->height / 2) - thickness / 2, (int)size->width,
              thickness, DEP_SPLIT, alpha);
}

static void draw_empty_cells(Image *image, const CellSize *size, float x, float y, int count,
                             int alpha)
{
    int i;

    for (i = 0; i < count; i++) {
        draw_cell(image, size, x + i * cell_step(size), y, alpha);
        draw_split(image, size, x + i * cell_step(size), y, alpha);
    }
}

static char scramble(UINT32 seed)
{
    static const char glyphs[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    seed ^= seed >> 13;
    seed *= 0x5bd1e995;
    seed ^= seed >> 15;
    return glyphs[seed % 36];
}

/* `text` upper-cased into `cells` characters, padded on the right or left. */
static void fit_cells(char *output, const char *text, int cells, int align_right)
{
    char upper[96];
    int length;
    int pad;
    int i;

    str_to_upper(upper, text, sizeof(upper));
    length = (int)str_length(upper);
    if (length > cells) {
        length = cells;
    }

    pad = 0;
    if (align_right) {
        pad = cells - length;
    }

    for (i = 0; i < cells; i++) {
        output[i] = ' ';
    }
    for (i = 0; i < length; i++) {
        output[pad + i] = upper[i];
    }
    output[cells] = '\0';
}

/*
 * Draws `text` into cells that are already on screen. Cells past `settled`
 * (0..1, left to right) still show random letters, flipping every FLIP_MS.
 */
static void draw_flaps(Image *screen, CellSize *size, float x, float y, const char *text,
                       float settled, UINT32 seed, float now, UINT32 color, int alpha)
{
    int count = (int)str_length(text);
    UINT32 tick = (UINT32)(now / FLIP_MS);
    int i;

    for (i = 0; i < count; i++) {
        float left = x + i * cell_step(size);
        char glyph[2];

        glyph[0] = text[i];
        glyph[1] = '\0';

        if ((i + 1.0f) / count > settled) {
            glyph[0] = scramble(seed * 977u + (UINT32)i * 131u + tick);
            draw_cell(screen, size, left, y, alpha * 120 / 256);
        }

        if (glyph[0] != ' ') {
            kit_text_center(screen, &size->face, left + size->width / 2,
                            (int)(y + (size->height + size->cap) / 2), glyph, color, alpha);
            draw_split(screen, size, left, y, alpha * 150 / 256);
        }
    }
}

static void short_version(char *output, int capacity, const char *title)
{
    const char *cursor = title;
    int length = 0;

    while (*cursor != '\0' && (*cursor < '0' || *cursor > '9')) {
        cursor++;
    }

    while (*cursor != '\0' && length < capacity - 1) {
        char c = *cursor;
        int digit = c >= '0' && c <= '9';
        int dash = c == '-' && cursor[1] >= '0' && cursor[1] <= '9';

        if (!digit && c != '.' && !dash) {
            break;
        }
        output[length++] = c;
        cursor++;
    }

    while (length > 0 && (output[length - 1] == '.' || output[length - 1] == '-')) {
        length--;
    }
    output[length] = '\0';

    if (length == 0) {
        str_copy(output, title, capacity);
    }
}

static void kernel_text(char *output, int capacity, const Entry *entry)
{
    output[0] = '\0';

    if (entry->action == ENTRY_ACTION_FIRMWARE) {
        str_copy(output, "UEFI", capacity);
    } else if (entry_has_kernels(entry)) {
        short_version(output, capacity, entry->kernels[0].title);
    }
}

/* The seconds in "Starting in 7", or -1 when the status is not a countdown. */
static int countdown(const char *status)
{
    int length = (int)str_length(status);
    int start = length;

    while (start > 0 && status[start - 1] >= '0' && status[start - 1] <= '9') {
        start--;
    }

    if (start == length || !str_starts_with_nocase(status, "Starting")) {
        return -1;
    }
    return str_to_int(status + start);
}

static int has_problem(const ThemeFrame *frame)
{
    return frame->status[0] != '\0' && countdown(frame->status) < 0;
}

static const char *status_text(const Entry *entry, const ThemeFrame *frame, int selected,
                               UINT32 *color)
{
    *color = DEP_TEXT;

    if (selected && frame->launching) {
        *color = DEP_SKY;
        return "DEPARTED";
    }
    if (selected && has_problem(frame)) {
        *color = DEP_DELAY;
        return "DELAYED";
    }
    if (selected) {
        *color = DEP_SKY;
        return "BOARDING";
    }
    if (!entry_is_system(entry)) {
        *color = DEP_MUTED;
        return "READY";
    }
    return "ON TIME";
}

static void paint_plane(Image *image, float center_x, float center_y, float size, UINT32 color)
{
    static const float outline[][2] = {
        { 0.00f, -1.00f }, { 0.09f, -0.85f }, { 0.09f, -0.25f }, { 0.85f, 0.15f },
        { 0.85f, 0.30f }, { 0.09f, 0.08f }, { 0.09f, 0.60f }, { 0.32f, 0.78f },
        { 0.32f, 0.90f }, { 0.00f, 0.82f }, { -0.32f, 0.90f }, { -0.32f, 0.78f },
        { -0.09f, 0.60f }, { -0.09f, 0.08f }, { -0.85f, 0.30f }, { -0.85f, 0.15f },
        { -0.09f, -0.25f }, { -0.09f, -0.85f }
    };
    Point points[18];
    float turn = 0.7071f;
    int i;

    for (i = 0; i < 18; i++) {
        float x = outline[i][0];
        float y = outline[i][1];

        points[i].x = center_x + (x * turn - y * turn) * size / 2;
        points[i].y = center_y + (x * turn + y * turn) * size / 2;
    }

    draw_polygon(image, points, 18, color, 256);
}

static void paint_header(Image *backdrop, const Image *screen)
{
    float box = kit_s(screen, 52);
    float x = kit_x(screen, LEFT);
    float y = kit_y(screen, 46);

    draw_round_rect(backdrop, x, y, box, box, box * 0.2f, 0, DEP_SKY, 256);
    paint_plane(backdrop, x + box / 2, y + box / 2, box * 0.62f, 0x0d0f12);

    draw_text(backdrop, &board.label, x + box + kit_s(screen, 18),
              (int)(y + kit_s(screen, 4) + board.label.ascent), "AARAMBH", DEP_SKY, 256);
    draw_text(backdrop, &board.title, x + box + kit_s(screen, 16),
              (int)(y + box + kit_s(screen, 2)), "Departures", DEP_TEXT, 256);

    kit_text_right(backdrop, &board.label, kit_x(screen, RIGHT), (int)(y + board.label.ascent),
                   "LOCAL TIME", DEP_MUTED, 256);
    draw_empty_cells(backdrop, &board.clock,
                     kit_x(screen, RIGHT) - CLOCK_CELLS * cell_step(&board.clock) +
                         (cell_step(&board.clock) - board.clock.width),
                     y + kit_s(screen, 24), CLOCK_CELLS, 256);
}

static void paint_columns(Image *backdrop, const Image *screen)
{
    int baseline = (int)(kit_y(screen, BOARD_TOP - 30) + board.label.ascent / 2);

    draw_text(backdrop, &board.label, kit_x(screen, NAME_X), baseline, "SYSTEM", DEP_LABEL, 256);
    draw_text(backdrop, &board.label, kit_x(screen, KERNEL_X), baseline, "KERNEL", DEP_LABEL, 256);
    draw_text(backdrop, &board.label, kit_x(screen, STATUS_X), baseline, "STATUS", DEP_LABEL, 256);
    draw_text(backdrop, &board.label, kit_x(screen, GATE_X), baseline, "GATE", DEP_LABEL, 256);
    draw_rect(backdrop, (int)kit_x(screen, LEFT), (int)kit_y(screen, BOARD_TOP - 14),
              (int)(kit_x(screen, RIGHT) - kit_x(screen, LEFT)), 1, DEP_RULE, 256);
}

static void paint_rows(Image *backdrop, const Image *screen, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        float top = row_top(screen, (float)i);
        float big_y = top + (board.row_height - board.big.height) / 2;
        float small_y = top + (board.row_height - board.small.height) / 2;

        draw_empty_cells(backdrop, &board.big, kit_x(screen, NAME_X), big_y, NAME_CELLS, 256);
        draw_empty_cells(backdrop, &board.small, kit_x(screen, KERNEL_X), small_y, KERNEL_CELLS, 256);
        draw_empty_cells(backdrop, &board.small, kit_x(screen, STATUS_X), small_y, STATUS_CELLS, 256);
        draw_empty_cells(backdrop, &board.big, kit_x(screen, GATE_X), big_y, GATE_CELLS, 256);
    }
}

static void paint_footer(Image *backdrop, const Image *screen)
{
    float y = kit_y(screen, FOOTER_Y);

    draw_text(backdrop, &board.text, kit_x(screen, LEFT + 30),
              (int)(y + board.text.ascent * 0.36f), "NOW BOARDING", DEP_DIM, 256);
    draw_empty_cells(backdrop, &board.small, board.footer_cells_x, y - board.small.height / 2,
                     NAME_CELLS, 256);
}

static void build_backdrop(const ThemeContext *context)
{
    const Image *screen = context->screen;
    Image *backdrop = &board.backdrop;

    if (!kit_backdrop(backdrop, screen, 0x1a1e25, 0x0d0f12)) {
        return;
    }

    kit_glow(backdrop, &board.glow, kit_x(screen, 380), kit_y(screen, 40), kit_s(screen, 1400),
             kit_s(screen, 700), 0x2a3340, 120);

    if (image_alloc(&board.plain, screen->width, screen->height)) {
        image_copy(&board.plain, backdrop);
    }

    paint_header(backdrop, screen);
    paint_columns(backdrop, screen);
    paint_rows(backdrop, screen, context->entry_count);
    paint_footer(backdrop, screen);
}

static void departures_load(const ThemeContext *context)
{
    const Image *screen = context->screen;
    float available = kit_s(screen, BOARD_BOTTOM - BOARD_TOP);
    float big;

    board.row_height = kit_s(screen, ROW_MAX);
    if (context->entry_count * board.row_height > available) {
        board.row_height = available / context->entry_count;
    }

    face_init(&board.label, context->font_regular, kit_s(screen, 13));
    face_init(&board.title, context->font_bold, kit_s(screen, 34));
    face_init(&board.text, context->font_regular, kit_s(screen, 17));
    face_init(&board.hint, context->font_regular, kit_s(screen, 14));
    face_init(&board.list, context->font_regular, kit_s(screen, 19));
    face_init(&board.splash, context->font_bold, kit_s(screen, 46));
    face_init(&board.detail, context->font_regular, kit_s(screen, 18));

    big = board.row_height * 0.64f;
    if (NAME_CELLS * big * (0.74f + CELL_GAP / 40.0f) > kit_s(screen, KERNEL_X - NAME_X - 30)) {
        big = kit_s(screen, KERNEL_X - NAME_X - 30) / (NAME_CELLS * (0.74f + CELL_GAP / 40.0f));
    }

    size_cells(&board.big, context->font_bold, big);
    size_cells(&board.small, context->font_bold, board.row_height * 0.5f);
    size_cells(&board.clock, context->font_bold, kit_s(screen, 36));

    board.footer_cells_x = kit_x(screen, LEFT + 30) + text_width(&board.text, "NOW BOARDING") +
                           kit_s(screen, 16);

    kit_glow_sprite(&board.glow);
    build_backdrop(context);
}

static float intro_settled(const ThemeFrame *frame, int row)
{
    return clampf((frame->intro_elapsed - row * INTRO_ROW_MS) / INTRO_FLIP_MS, 0.0f, 1.0f);
}

static void draw_highlight(Image *screen, const ThemeFrame *frame, int alpha)
{
    float top = row_top(screen, frame->position);
    float left = kit_x(screen, LEFT);
    float width = kit_x(screen, RIGHT) - left;
    int steps = 32;
    int i;

    for (i = 0; i < steps; i++) {
        int x = (int)(left + width * i / steps);
        int next = (int)(left + width * (i + 1) / steps);

        draw_rect(screen, x, (int)top, next - x, (int)board.row_height, DEP_SKY,
                  alpha * 40 / 256 * (steps - i) / steps);
    }

    draw_round_rect(screen, left, top + board.row_height * 0.14f, kit_s(screen, 5),
                    board.row_height * 0.72f, kit_s(screen, 2.5f), 0, DEP_SKY, alpha);
}

static void draw_row(const ThemeContext *context, const ThemeFrame *frame, int index, int alpha)
{
    Image *screen = context->screen;
    const Entry *entry = &context->entries[index];
    float weight = kit_weight(frame, index);
    float intro = intro_settled(frame, index);
    float flip = absf(weight - 0.5f) * 2.0f;
    float top = row_top(screen, (float)index);
    float big_y = top + (board.row_height - board.big.height) / 2;
    float small_y = top + (board.row_height - board.small.height) / 2;
    float logo = board.row_height * 0.46f;
    UINT32 name_color = DEP_DIM;
    UINT32 status_color;
    const char *status;
    char cells[TEXT_CELLS + 1];
    char kernel[48];
    char gate[8];

    if (weight > 0.5f) {
        name_color = DEP_TEXT;
    }

    kit_logo(screen, entry, &board.big.face, kit_x(screen, LOGO_X), top + (board.row_height - logo) / 2,
             logo, DEP_TEXT, (int)(alpha * (0.5f + 0.5f * weight)));

    fit_cells(cells, entry->title, NAME_CELLS, 0);
    draw_flaps(screen, &board.big, kit_x(screen, NAME_X), big_y, cells, intro, (UINT32)index,
               frame->now, name_color, alpha);

    kernel_text(kernel, sizeof(kernel), entry);
    fit_cells(cells, kernel, KERNEL_CELLS, 0);
    draw_flaps(screen, &board.small, kit_x(screen, KERNEL_X), small_y, cells, intro,
               (UINT32)index + 40u, frame->now, DEP_MUTED, alpha);

    status = status_text(entry, frame, weight > 0.5f, &status_color);
    fit_cells(cells, status, STATUS_CELLS, 0);
    draw_flaps(screen, &board.small, kit_x(screen, STATUS_X), small_y, cells,
               intro < flip ? intro : flip, (UINT32)index + 80u, frame->now, status_color, alpha);

    gate[0] = '\0';
    if (index < 9) {
        str_copy(gate, "0", sizeof(gate));
    }
    str_append_int(gate, index + 1, sizeof(gate));
    draw_flaps(screen, &board.big, kit_x(screen, GATE_X), big_y, gate, intro, (UINT32)index + 120u,
               frame->now, weight > 0.5f ? DEP_SKY : DEP_DIM, alpha);
}

static void draw_clock(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    char text[24];
    char cells[CLOCK_CELLS + 1];

    str_copy(text, frame->clock_time, sizeof(text));
    if (frame->clock_period[0] != '\0') {
        str_append(text, " ", sizeof(text));
        str_append(text, frame->clock_period, sizeof(text));
    }
    fit_cells(cells, text, CLOCK_CELLS, 1);

    draw_flaps(screen, &board.clock,
               kit_x(screen, RIGHT) - CLOCK_CELLS * cell_step(&board.clock) +
                   (cell_step(&board.clock) - board.clock.width),
               kit_y(screen, 46) + kit_s(screen, 24), cells, intro_settled(frame, 0), 7u,
               frame->now, DEP_TEXT, alpha);
}

static void draw_footer(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    float visibility;
    int index = kit_nearest(frame, context->entry_count, &visibility);
    const Entry *entry = &context->entries[index];
    float y = kit_y(screen, FOOTER_Y);
    float dot = kit_s(screen, 12);
    float x = board.footer_cells_x + NAME_CELLS * cell_step(&board.small) + kit_s(screen, 14);
    int seconds = countdown(frame->status);
    int baseline = (int)(y + board.text.ascent * 0.36f);
    char cells[TEXT_CELLS + 1];

    kit_glow(screen, &board.glow, kit_x(screen, LEFT) + dot / 2, y, dot * 3, dot * 3, DEP_SKY,
             alpha * 120 / 256);
    draw_round_rect(screen, kit_x(screen, LEFT), y - dot / 2, dot, dot, dot / 2, 0, DEP_SKY, alpha);

    fit_cells(cells, entry->title, NAME_CELLS, 0);
    draw_flaps(screen, &board.small, board.footer_cells_x, y - board.small.height / 2, cells,
               visibility < intro_settled(frame, 2) ? visibility : intro_settled(frame, 2),
               (UINT32)index + 200u, frame->now, DEP_TEXT, alpha);

    if (frame->launching) {
        draw_text(screen, &board.text, x, baseline, "NOW DEPARTING", DEP_SKY, alpha);
    } else if (seconds >= 0) {
        draw_text(screen, &board.text, x, baseline, "DEPARTS IN", DEP_DIM, alpha);
        x += text_width(&board.text, "DEPARTS IN") + kit_s(screen, 12);
        cells[0] = (char)('0' + (seconds / 10) % 10);
        cells[1] = (char)('0' + seconds % 10);
        cells[2] = '\0';
        draw_empty_cells(screen, &board.small, x, y - board.small.height / 2, COUNT_CELLS, alpha);
        draw_flaps(screen, &board.small, x, y - board.small.height / 2, cells, 1.0f, 0u,
                   frame->now, DEP_SKY, alpha);
        draw_text(screen, &board.text, x + COUNT_CELLS * cell_step(&board.small) + kit_s(screen, 10),
                  baseline, "SECONDS", DEP_DIM, alpha);
    } else if (frame->status[0] != '\0') {
        draw_text(screen, &board.text, x, baseline, frame->status, DEP_DELAY, alpha);
    }
}

static void draw_kernel_panel(const ThemeContext *context, const ThemeFrame *frame, int alpha)
{
    Image *screen = context->screen;
    const Entry *entry = &context->entries[frame->selected];
    float x = kit_x(screen, 690);
    float top = kit_y(screen, 150);
    float width = kit_x(screen, RIGHT) - x;
    int shown = (int)(alpha * frame->list);

    if (frame->list <= 0.0f || !entry_has_kernels(entry)) {
        return;
    }

    draw_rect(screen, 0, 0, screen->width, screen->height, 0x0d0f12, shown * 150 / 256);
    draw_round_rect(screen, x, top, width, kit_s(screen, 330), kit_s(screen, 14), 0, 0x15181d,
                    shown);
    draw_round_rect(screen, x, top, width, kit_s(screen, 330), kit_s(screen, 14), 1, DEP_RULE,
                    shown);
    kit_kernel_list(screen, entry, frame, &board.label, &board.list, x + kit_s(screen, 26),
                    top + kit_s(screen, 26), width - kit_s(screen, 52), DEP_TEXT, DEP_SKY, alpha);
}

static void departures_render(const ThemeContext *context, const ThemeFrame *frame)
{
    Image *screen = context->screen;
    KitFade fade = kit_fade(frame);
    UiHintStyle hints = { &board.hint, DEP_MUTED, 0 };
    int i;

    kit_draw_backdrop(screen, &board.backdrop);
    draw_highlight(screen, frame, fade.alpha);

    for (i = 0; i < context->entry_count; i++) {
        draw_row(context, frame, i, fade.alpha);
    }

    draw_clock(context, frame, fade.alpha);
    draw_footer(context, frame, fade.alpha);
    draw_kernel_panel(context, frame, fade.alpha);

    kit_hints(screen, &hints, kit_x(screen, LEFT), (int)(screen->height - kit_s(screen, 40)),
              &context->entries[frame->selected], frame, 1, fade.alpha * 200 / 256);

    kit_splash(context, frame, &board.splash, &board.detail, &board.plain, DEP_TEXT, DEP_SKY);
    kit_intro_blackout(screen, frame);
}

static const EditColors departures_edit = { 0x15181d, 0x0d0f12, DEP_SKY, DEP_TEXT, DEP_MUTED,
                                            EDIT_FRAME_CARD };

const Theme departures_theme = {
    .name = "departures",
    .list_rows = KIT_LIST_ROWS,
    .vertical_navigation = 1,
    .down_opens_list = 0,
    .edit = &departures_edit,
    .load = departures_load,
    .render = departures_render
};