#include <efi.h>
#include <efilib.h>

#include "core/clock.h"

#include "lib/memory.h"

#define MINUTES_PER_DAY 1440
#define CLOCK_REFRESH_MS 1000.0f

void clock_init(Clock *clock, int utc_offset_minutes)
{
    mem_set(clock, 0, sizeof(*clock));
    clock->utc_offset_minutes = utc_offset_minutes;
}

static int local_minutes(const EFI_TIME *time, int offset)
{
    int minutes = time->Hour * 60 + time->Minute + offset;

    minutes %= MINUTES_PER_DAY;
    if (minutes < 0) {
        minutes += MINUTES_PER_DAY;
    }

    return minutes;
}

static void format_time(Clock *clock, int hour, int minute)
{
    int display_hour = hour % 12;
    int i = 0;

    if (display_hour == 0) {
        display_hour = 12;
    }

    if (display_hour >= 10) {
        clock->time[i] = (char)('0' + display_hour / 10);
        i++;
    }

    clock->time[i++] = (char)('0' + display_hour % 10);
    clock->time[i++] = ':';
    clock->time[i++] = (char)('0' + minute / 10);
    clock->time[i++] = (char)('0' + minute % 10);
    clock->time[i] = '\0';

    if (hour < 12) {
        clock->period[0] = 'A';
    } else {
        clock->period[0] = 'P';
    }
    clock->period[1] = 'M';
    clock->period[2] = '\0';
}

void clock_update(Clock *clock, float now)
{
    EFI_TIME time;
    int minutes;

    if (clock->time[0] != '\0' && now - clock->updated_at < CLOCK_REFRESH_MS) {
        return;
    }

    clock->updated_at = now;

    if (RT->GetTime(&time, NULL) != EFI_SUCCESS) {
        return;
    }

    minutes = local_minutes(&time, clock->utc_offset_minutes);
    format_time(clock, minutes / 60, minutes % 60);
}

int clock_parse_offset(const char *text)
{
    int sign = 1;
    int digits[4] = { 0 };
    int i;

    if (*text == '-') {
        sign = -1;
    }

    if (*text == '+' || *text == '-') {
        text++;
    }

    for (i = 0; i < 4 && text[i] >= '0' && text[i] <= '9'; i++) {
        digits[i] = text[i] - '0';
    }

    return sign * ((digits[0] * 10 + digits[1]) * 60 + digits[2] * 10 + digits[3]);
}
