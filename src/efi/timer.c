#include <efi.h>
#include <efilib.h>

#include "efi/timer.h"

#define CALIBRATION_US 50000
#define CALIBRATION_MS 50.0

static UINT64 start_ticks;
static double ticks_per_ms;

static inline UINT64 read_tsc(void)
{
    UINT32 low;
    UINT32 high;

    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((UINT64)high << 32) | low;
}

void timer_calibrate(void)
{
    UINT64 before = read_tsc();

    BS->Stall(CALIBRATION_US);
    ticks_per_ms = (double)(read_tsc() - before) / CALIBRATION_MS;
    start_ticks = read_tsc();
}

float timer_now_ms(void)
{
    return (float)((double)(read_tsc() - start_ticks) / ticks_per_ms);
}

void timer_sleep_ms(float milliseconds)
{
    if (milliseconds > 0.0f) {
        BS->Stall((UINTN)(milliseconds * 1000.0f));
    }
}
