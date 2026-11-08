#ifndef AARAMBH_CORE_TWEEN_H
#define AARAMBH_CORE_TWEEN_H

typedef struct {
    float from;
    float to;
    float start;
    float duration;
} Tween;

void tween_init(Tween *tween, float value, float duration);
void tween_reset(Tween *tween, float value);
void tween_to(Tween *tween, float target, float now);
float tween_value(const Tween *tween, float now);

#endif
