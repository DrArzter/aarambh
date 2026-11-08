#include "themes/accordion/accordion.h"

#define CONTENT_LEFT        0.065f
#define CONTENT_RIGHT       0.05f
#define CONTENT_SLIDE       0.04f
#define LOGO_SIZE           0.15f
#define LOGO_Y              0.22f
#define LOGO_LIST_SHIFT     0.06f
#define LETTER_LIFT         0.02f
#define TITLE_BASELINE      0.535f
#define TITLE_LIST_SHIFT    0.09f
#define TITLE_NUDGE         0.006f
#define SUBTITLE_BASELINE   0.6f
#define SUBTITLE_COLOR      0xe8e4ee
#define BUTTON_Y            0.655f
#define BUTTON_HEIGHT       0.068f
#define BUTTON_SPACING      0.02f
#define LIST_TOP            0.53f
#define LIST_DROP           0.05f
#define LIST_ROW_HEIGHT     0.056f
#define STATUS_BASELINE     0.82f
#define STATUS_ALPHA        210

typedef struct {
    Image *screen;
    const Entry *entry;
    float width;
    float height;
    float left;
    float right;
    float list;
    float focus;
    int alpha;
    int content_alpha;
} PanelLayout;

static void draw_logo(const PanelLayout *panel)
{
    int size = (int)(panel->height * LOGO_SIZE);
    int y = (int)(panel->height * LOGO_Y - panel->list * panel->height * LOGO_LIST_SHIFT);
    char letter[2];

    if (entry_has_logo(panel->entry)) {
        draw_image(panel->screen, &panel->entry->logo, (int)panel->left, y, size, size,
                   panel->alpha);
        return;
    }

    letter[0] = panel->entry->title[0];
    letter[1] = '\0';
    draw_text_shadow(panel->screen, &accordion_faces.letter, panel->left,
                     y + size - (int)(panel->height * LETTER_LIFT), letter,
                     COLOR_WHITE, panel->alpha);
}

static void draw_title(const PanelLayout *panel)
{
    float shift = panel->list * panel->height * TITLE_LIST_SHIFT;
    int baseline = (int)(panel->height * TITLE_BASELINE - shift);
    float x = panel->left - panel->height * TITLE_NUDGE;

    draw_text_shadow(panel->screen, &accordion_faces.title, x, baseline, panel->entry->title,
                     COLOR_WHITE, panel->alpha);
}

static void draw_subtitle(const PanelLayout *panel, int is_default)
{
    char subtitle[128];
    int baseline = (int)(panel->height * SUBTITLE_BASELINE);

    entry_subtitle(panel->entry, is_default, subtitle, sizeof(subtitle));
    draw_text_shadow(panel->screen, &accordion_faces.subtitle, panel->left, baseline, subtitle,
                     SUBTITLE_COLOR, panel->content_alpha);
}

static void draw_buttons(const PanelLayout *panel)
{
    UiButton primary;
    UiButton advanced;
    float primary_end;

    primary.x = panel->left;
    primary.y = panel->height * BUTTON_Y;
    primary.height = panel->height * BUTTON_HEIGHT;
    primary.label = entry_action_label(panel->entry);
    primary.play_icon = 1;
    primary.active = 1.0f - panel->focus;
    primary.idle_fill = 0;
    primary.border = 256;
    primary.fill_color = COLOR_WHITE;
    primary.text_color = COLOR_INK;

    primary_end = ui_button_draw(panel->screen, &accordion_faces.button, &primary,
                                 panel->content_alpha);

    if (!entry_has_kernels(panel->entry)) {
        return;
    }

    advanced = primary;
    advanced.x = primary_end + panel->height * BUTTON_SPACING;
    advanced.label = "Advanced options";
    advanced.play_icon = 0;
    advanced.active = panel->focus;
    advanced.idle_fill = 40;
    advanced.border = 150;

    ui_button_draw(panel->screen, &accordion_faces.button, &advanced, panel->content_alpha);
}

static void draw_kernel_list(const PanelLayout *panel, const ThemeFrame *frame)
{
    UiOptionList list;

    if (panel->list <= 0.0f || !entry_has_kernels(panel->entry)) {
        return;
    }

    list.heading_face = &accordion_faces.hint;
    list.row_face = &accordion_faces.list;
    list.heading = "ADVANCED OPTIONS";
    list.x = panel->left;
    list.top = panel->height * LIST_TOP + (1.0f - panel->list) * panel->height * LIST_DROP;
    list.width = panel->right - panel->left;
    list.row_height = panel->height * LIST_ROW_HEIGHT;
    list.rows = ACCORDION_LIST_ROWS;
    list.heading_color = 0xd0c8dc;
    list.text_color = COLOR_WHITE;
    list.highlight_color = COLOR_WHITE;
    list.accent_color = UI_ENTRY_ACCENT;
    list.shadow = 1;

    ui_option_list_draw(panel->screen, &list, panel->entry, frame,
                        (int)(panel->alpha * panel->list));
}

static void draw_status(const PanelLayout *panel, const ThemeFrame *frame)
{
    int baseline = (int)(panel->height * STATUS_BASELINE);
    int alpha = (int)(panel->alpha * (1.0f - panel->list)) * STATUS_ALPHA / 256;

    if (frame->status[0] == '\0') {
        return;
    }

    draw_text_shadow(panel->screen, &accordion_faces.hint, panel->left, baseline, frame->status,
                     COLOR_WHITE, alpha);
}

void accordion_draw_main_panel(const ThemeContext *context, const ThemeFrame *frame,
                               int index, const AccordionColumn *column, int alpha)
{
    PanelLayout panel;
    float slide;

    if (alpha <= 0) {
        return;
    }

    panel.screen = context->screen;
    panel.entry = &context->entries[index];
    panel.width = (float)context->screen->width;
    panel.height = (float)context->screen->height;
    panel.list = clampf(frame->list, 0.0f, 1.0f);
    panel.focus = clampf(frame->focus, 0.0f, 1.0f);
    panel.alpha = alpha;
    panel.content_alpha = (int)(alpha * (1.0f - panel.list));

    slide = (1.0f - alpha / 256.0f) * panel.width * CONTENT_SLIDE;
    panel.left = column->x + panel.width * CONTENT_LEFT + slide;
    panel.right = column->x + column->width - panel.width * CONTENT_RIGHT;

    draw_logo(&panel);
    draw_title(&panel);
    draw_subtitle(&panel, index == context->default_index);
    draw_buttons(&panel);
    draw_kernel_list(&panel, frame);
    accordion_draw_hint_bar(context, frame, panel.entry, panel.left, alpha);
    draw_status(&panel, frame);
}
