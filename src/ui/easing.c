#include "aarambh/easing.h"

float clampf(float value, float low, float high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

float absf(float value)
{
    if (value < 0) {
        return -value;
    }
    return value;
}

float ease_out_cubic(float progress)
{
    float remaining;

    progress = clampf(progress, 0.0f, 1.0f);
    remaining = 1.0f - progress;

    return 1.0f - remaining * remaining * remaining;
}

float ease_in_out_cubic(float progress)
{
    float remaining;

    progress = clampf(progress, 0.0f, 1.0f);

    if (progress < 0.5f) {
        return 4.0f * progress * progress * progress;
    }

    remaining = -2.0f * progress + 2.0f;
    return 1.0f - remaining * remaining * remaining / 2.0f;
}
