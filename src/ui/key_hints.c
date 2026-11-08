#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/widgets.h"

#define KEY_CAP_PADDING     9.0f
#define KEY_CAP_SPACING     10.0f
#define KEY_CAP_RADIUS      6.0f
#define KEY_CAP_BORDER      1.6f
#define KEY_CAP_BORDER_ALPHA 200
#define ARROW_SPACING       14.0f

static void hint_text(Image *screen, const UiHintStyle *style, float x, int baseline,
                      const char *label, int alpha)
{
    if (style->shadow) {
        draw_text_shadow(screen, style->face, x, baseline, label, style->color, alpha);
    } else {
        draw_text(screen, style->face, x, baseline, label, style->color, alpha);
    }
}

float ui_key_cap(Image *screen, const UiHintStyle *style, float x, int baseline,
                 const char *label, int alpha)
{
    Face *face = style->face;
    float label_width = text_width(face, label);
    float width = label_width + KEY_CAP_PADDING * 2.0f;
    float height = (float)face_line_height(face) + 8.0f;
    float y = baseline - face->ascent - 5.0f;
    int border_alpha = alpha * KEY_CAP_BORDER_ALPHA / 256;

    draw_round_rect(screen, x, y, width, height, KEY_CAP_RADIUS, KEY_CAP_BORDER,
                    style->color, border_alpha);
    draw_text(screen, face, x + KEY_CAP_PADDING, baseline, label, style->color, alpha);

    return x + width + KEY_CAP_SPACING;
}

static float draw_horizontal_arrows(Image *screen, int size, float x, int baseline,
                                    UINT32 color, int alpha)
{
    int top = baseline - size;

    draw_triangle(screen, (int)x, top, size, TRIANGLE_LEFT, color, alpha);
    draw_triangle(screen, (int)x + size + 4, top, size, TRIANGLE_RIGHT, color, alpha);

    return x + size * 2 + ARROW_SPACING;
}

static float draw_vertical_arrows(Image *screen, int size, float x, int baseline,
                                  UINT32 color, int alpha)
{
    int height = size * 6 / 10;
    int top = baseline - size + (size - height) / 2;
    int up_x = (int)x;
    int down_x = up_x + height * 2 + 6;
    int i;

    for (i = 0; i < height; i++) {
        int row_width = i * 2 + 1;

        draw_rect(screen, up_x + height - i, top + i, row_width, 1, color, alpha);
        draw_rect(screen, down_x + height - i, top + height - 1 - i, row_width, 1, color, alpha);
    }

    return down_x + height * 2 + ARROW_SPACING;
}

float ui_arrows(Image *screen, const UiHintStyle *style, float x, int baseline,
                UiArrowAxis axis, int alpha)
{
    int size = style->face->ascent;

    if (axis == UI_ARROWS_VERTICAL) {
        return draw_vertical_arrows(screen, size, x, baseline, style->color, alpha);
    }
    return draw_horizontal_arrows(screen, size, x, baseline, style->color, alpha);
}

float ui_hint(Image *screen, const UiHintStyle *style, float x, int baseline,
              const char *label, int alpha)
{
    hint_text(screen, style, x, baseline, label, alpha);

    return x + text_width(style->face, label) + style->face->ascent * 2.0f;
}
