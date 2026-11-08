#ifndef AARAMBH_CORE_SHUTDOWN_SPLASH_H
#define AARAMBH_CORE_SHUTDOWN_SPLASH_H

#include "aarambh/entry.h"
#include "aarambh/gfx/image.h"

/* Renders the launch splash still with `title` ("" for none) into the screen. */
typedef void (*StillRenderer)(const char *title);

/*
 * Restart and shutdown splashes come from the OS's own disk, out of
 * Aarambh's reach at that moment. So when a Linux entry starts and the theme
 * or resolution changed since last time, this writes that OS's splash images
 * to \EFI\aarambh\shutdown\: <key>-bg.png (the still without a title),
 * <key>-reboot.png and <key>-shutdown.png (the title rows) and <key>-dot.png.
 * aarambh-clear copies them into the installed Plymouth theme at boot.
 */
void shutdown_splash_update(const Entry *entry, const char *theme, const Image *screen,
                            StillRenderer render);

#endif