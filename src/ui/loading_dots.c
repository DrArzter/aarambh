#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/widgets.h"

#define DOT_COUNT     3
#define DOT_STEP_MS   260.0f
#define DOT_RADIUS    0.0068f
#define DOT_SPACING   0.028f
#define DOT_DIM_ALPHA 110

void ui_loading_dots(Image *screen, float center_x, float center_y, float now,
                     UINT32 color, int alpha)
{
    float unit = (float)screen->height;
    float radius = unit * DOT_RADIUS;
    int active = (int)(now / DOT_STEP_MS) % DOT_COUNT;
    int i;

    for (i = 0; i < DOT_COUNT; i++) {
        float x = center_x + (i - 1) * unit * DOT_SPACING;
        int dot_alpha = alpha * DOT_DIM_ALPHA / 256;

        if (i == active) {
            dot_alpha = alpha;
        }

        draw_round_rect(screen, x - radius, center_y - radius, radius * 2, radius * 2, radius, 0,
                        color, dot_alpha);
    }
}
