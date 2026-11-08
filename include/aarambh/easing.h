#ifndef AARAMBH_EASING_H
#define AARAMBH_EASING_H

float clampf(float value, float low, float high);
float absf(float value);

/* Both curves clamp their input to 0..1 and return a value in 0..1. */
float ease_out_cubic(float progress);
float ease_in_out_cubic(float progress);

#endif
