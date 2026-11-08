#include "aarambh/easing.h"
#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/widgets.h"

#define HEADING_OFFSET       0.018f
#define ROW_GAP              6.0f
#define ROW_RADIUS           0.012f
#define ROW_HIGHLIGHT_ALPHA  46
#define ROW_DIM_ALPHA        170
#define ROW_TEXT_INDENT      0.03f
#define ROW_SLIDE_DISTANCE   0.04f
#define ACCENT_BAR_WIDTH     0.005f
#define STAGGER_SPEED        1.6f
#define STAGGER_DELAY        0.1f
#define BASELINE_SHIFT       0.38f

static int row_is_visible(const UiOptionList *list, float row_y)
{
    float last_row_y = list->top + (list->rows - 1) * list->row_height;

    return row_y >= list->top - 1.0f && row_y <= last_row_y + 1.0f;
}

static UINT32 accent_for(const UiOptionList *list, const Entry *entry)
{
    if (list->accent_color == UI_ENTRY_ACCENT) {
        return entry->accent;
    }
    return list->accent_color;
}

static void draw_row_highlight(Image *screen, const UiOptionList *list, const Entry *entry,
                               float x, float y, int alpha)
{
    float unit = (float)screen->height;
    int bar_width = (int)(unit * ACCENT_BAR_WIDTH) + 1;
    int bar_y = (int)(y + list->row_height * 0.2f);
    int bar_height = (int)(list->row_height * 0.5f);

    draw_round_rect(screen, x, y, list->width, list->row_height - ROW_GAP, unit * ROW_RADIUS, 0,
                    list->highlight_color, alpha * ROW_HIGHLIGHT_ALPHA / 256);
    draw_rect(screen, (int)x, bar_y, bar_width, bar_height, accent_for(list, entry), alpha);
}

static void draw_row(Image *screen, const UiOptionList *list, const Entry *entry,
                     const ThemeFrame *frame, int index, int alpha)
{
    float unit = (float)screen->height;
    float row_y = list->top + (index - frame->list_scroll) * list->row_height;
    float stagger;
    float row_x;
    int row_alpha;
    int text_alpha;
    int baseline;
    int selected = index == frame->list_selected;

    if (!row_is_visible(list, row_y)) {
        return;
    }

    stagger = clampf(frame->list * STAGGER_SPEED - index * STAGGER_DELAY, 0.0f, 1.0f);
    row_alpha = (int)(alpha * stagger);
    row_x = list->x + (1.0f - stagger) * unit * ROW_SLIDE_DISTANCE;

    if (selected) {
        draw_row_highlight(screen, list, entry, row_x, row_y, row_alpha);
    }

    baseline = (int)(row_y + (list->row_height - ROW_GAP) / 2
                     + list->row_face->ascent * BASELINE_SHIFT);

    text_alpha = row_alpha * ROW_DIM_ALPHA / 256;
    if (selected) {
        text_alpha = row_alpha;
    }

    draw_text(screen, list->row_face, row_x + unit * ROW_TEXT_INDENT, baseline,
              entry->kernels[index].title, list->text_color, text_alpha);
}

void ui_option_list_draw(Image *screen, const UiOptionList *list, const Entry *entry,
                         const ThemeFrame *frame, int alpha)
{
    int heading_baseline = (int)(list->top - screen->height * HEADING_OFFSET);
    int i;

    if (list->shadow) {
        draw_text_shadow(screen, list->heading_face, list->x, heading_baseline, list->heading,
                         list->heading_color, alpha);
    } else {
        draw_text(screen, list->heading_face, list->x, heading_baseline, list->heading,
                  list->heading_color, alpha);
    }

    for (i = 0; i < entry->kernel_count; i++) {
        draw_row(screen, list, entry, frame, i, alpha);
    }
}
