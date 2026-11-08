#ifndef AARAMBH_EFI_TIMER_H
#define AARAMBH_EFI_TIMER_H

#include <efi.h>

void timer_calibrate(void);
float timer_now_ms(void);
void timer_sleep_ms(float milliseconds);

#endif
