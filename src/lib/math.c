#include <efi.h>

#include "lib/math.h"

double math_floor(double x)
{
    double truncated = (double)(INT64)x;

    if (truncated > x) {
        return truncated - 1.0;
    }
    return truncated;
}

double math_ceil(double x)
{
    double truncated = (double)(INT64)x;

    if (truncated < x) {
        return truncated + 1.0;
    }
    return truncated;
}

double math_sqrt(double x)
{
    double result;

    __asm__("sqrtsd %1, %0" : "=x"(result) : "x"(x));
    return result;
}

double math_fabs(double x)
{
    if (x < 0) {
        return -x;
    }
    return x;
}

double math_fmod(double x, double y)
{
    return x - y * (double)(INT64)(x / y);
}

double math_pow(double x, double y)
{
    double result = 1.0;
    INT64 count = (INT64)y;

    while (count > 0) {
        result *= x;
        count--;
    }

    return result;
}

static double reduce_angle(double x)
{
    double turn = 2.0 * MATH_PI;

    x = math_fmod(x, turn);
    if (x > MATH_PI) {
        x -= turn;
    }
    if (x < -MATH_PI) {
        x += turn;
    }

    return x;
}

static double sin_taylor(double x)
{
    double x2 = x * x;

    return x * (1.0 - x2 / 6.0 * (1.0 - x2 / 20.0 * (1.0 - x2 / 42.0
               * (1.0 - x2 / 72.0 * (1.0 - x2 / 110.0)))));
}

double math_sin(double x)
{
    x = reduce_angle(x);

    if (x > MATH_PI / 2.0) {
        x = MATH_PI - x;
    } else if (x < -MATH_PI / 2.0) {
        x = -MATH_PI - x;
    }

    return sin_taylor(x);
}

double math_cos(double x)
{
    return math_sin(x + MATH_PI / 2.0);
}

static double atan_unit(double x)
{
    double x2 = x * x;

    return x * (0.9998660 + x2 * (-0.3302995 + x2 * (0.1801410
               + x2 * (-0.0851330 + x2 * 0.0208351))));
}

double math_atan2(double y, double x)
{
    double ay = math_fabs(y);
    double ax = math_fabs(x);
    double angle;

    if (ax == 0.0 && ay == 0.0) {
        return 0.0;
    }

    if (ax >= ay) {
        angle = atan_unit(ay / ax);
    } else {
        angle = MATH_PI / 2.0 - atan_unit(ax / ay);
    }

    if (x < 0) {
        angle = MATH_PI - angle;
    }
    if (y < 0) {
        angle = -angle;
    }

    return angle;
}

double math_acos(double x)
{
    double x3 = x * x * x;
    double x5 = x3 * x * x;

    return MATH_PI / 2.0 - x - x3 / 6.0 - 3.0 * x5 / 40.0;
}

int abs(int x)
{
    if (x < 0) {
        return -x;
    }
    return x;
}
