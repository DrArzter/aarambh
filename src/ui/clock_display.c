#include "aarambh/widgets.h"

static void clock_text(Image *screen, const UiClock *clock, Face *face, float x,
                       const char *text, UINT32 color, int alpha)
{
    if (clock->shadow) {
        draw_text_shadow(screen, face, x, clock->baseline, text, color, alpha);
    } else {
        draw_text(screen, face, x, clock->baseline, text, color, alpha);
    }
}

void ui_clock_draw(Image *screen, const UiClock *clock, const ThemeFrame *frame, int alpha)
{
    float period_width = text_width(clock->period_face, frame->clock_period);
    float time_width = text_width(clock->time_face, frame->clock_time);
    float period_x = clock->right - period_width;
    float time_x = period_x - clock->gap - time_width;

    clock_text(screen, clock, clock->period_face, period_x, frame->clock_period,
               clock->period_color, alpha);
    clock_text(screen, clock, clock->time_face, time_x, frame->clock_time,
               clock->time_color, alpha);
}
