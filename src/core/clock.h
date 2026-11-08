#ifndef AARAMBH_CORE_CLOCK_H
#define AARAMBH_CORE_CLOCK_H

typedef struct {
    char time[16];
    char period[4];
    int utc_offset_minutes;
    float updated_at;
} Clock;

void clock_init(Clock *clock, int utc_offset_minutes);
void clock_update(Clock *clock, float now);

int clock_parse_offset(const char *text);

#endif
