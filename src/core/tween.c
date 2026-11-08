#include "core/tween.h"

#include "aarambh/easing.h"

#define TWEEN_FINISHED -100000.0f

void tween_init(Tween *tween, float value, float duration)
{
    tween->duration = duration;
    tween_reset(tween, value);
}

void tween_reset(Tween *tween, float value)
{
    tween->from = value;
    tween->to = value;
    tween->start = TWEEN_FINISHED;
}

void tween_to(Tween *tween, float target, float now)
{
    tween->from = tween_value(tween, now);
    tween->to = target;
    tween->start = now;
}

float tween_value(const Tween *tween, float now)
{
    float progress = ease_in_out_cubic((now - tween->start) / tween->duration);

    return tween->from + (tween->to - tween->from) * progress;
}
