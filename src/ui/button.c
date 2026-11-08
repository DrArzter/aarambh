#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/widgets.h"

#define BUTTON_BORDER         2.5f
#define BUTTON_ICON_OFFSET    0.45f
#define BUTTON_ICON_SIZE      0.32f
#define BUTTON_ICON_SPAN      1.45f
#define BUTTON_PADDING        0.03f
#define BUTTON_ICON_GAP       0.016f
#define BUTTON_BASELINE_SHIFT 0.38f

void ui_play_icon(Image *screen, int x, int center_y, int size, UINT32 color, int alpha)
{
    draw_triangle(screen, x, center_y - size / 2, size, TRIANGLE_RIGHT, color, alpha);
}

float ui_button_width(const Image *screen, Face *face, const UiButton *button)
{
    float unit = (float)screen->height;
    float label_width = text_width(face, button->label);

    if (button->play_icon) {
        return label_width + button->height * BUTTON_ICON_SPAN + unit * BUTTON_PADDING;
    }
    return label_width + unit * BUTTON_PADDING * 2.0f;
}

static void draw_button_background(Image *screen, const UiButton *button, float width,
                                   int active, int alpha)
{
    float radius = button->height / 2;
    int idle = 256 - active;

    draw_round_rect(screen, button->x, button->y, width, button->height, radius, 0,
                    button->fill_color, alpha * active / 256);

    if (button->idle_fill > 0) {
        draw_round_rect(screen, button->x, button->y, width, button->height, radius, 0,
                        button->fill_color, alpha * button->idle_fill / 256 * idle / 256);
    }

    draw_round_rect(screen, button->x, button->y, width, button->height, radius,
                    BUTTON_BORDER, button->fill_color, alpha * button->border / 256);
}

static float draw_button_icon(Image *screen, const UiButton *button, int active, int alpha)
{
    int size = (int)(button->height * BUTTON_ICON_SIZE);
    int icon_x = (int)(button->x + button->height * BUTTON_ICON_OFFSET);
    int center_y = (int)(button->y + button->height / 2);
    int idle = 256 - active;

    ui_play_icon(screen, icon_x, center_y, size, button->text_color, alpha * active / 256);
    ui_play_icon(screen, icon_x, center_y, size, button->fill_color, alpha * idle / 256);

    return button->x + button->height * BUTTON_ICON_OFFSET + size
         + screen->height * BUTTON_ICON_GAP;
}

float ui_button_draw(Image *screen, Face *face, const UiButton *button, int alpha)
{
    float width = ui_button_width(screen, face, button);
    int active = (int)(256 * button->active);
    int idle = 256 - active;
    int baseline = (int)(button->y + button->height / 2 + face->ascent * BUTTON_BASELINE_SHIFT);
    float label_x = button->x + screen->height * BUTTON_PADDING;

    draw_button_background(screen, button, width, active, alpha);

    if (button->play_icon) {
        label_x = draw_button_icon(screen, button, active, alpha);
    }

    draw_text(screen, face, label_x, baseline, button->label, button->text_color,
              alpha * active / 256);
    draw_text(screen, face, label_x, baseline, button->label, button->fill_color,
              alpha * idle / 256);

    return button->x + width;
}
