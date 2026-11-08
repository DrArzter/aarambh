#ifndef AARAMBH_BOOT_PLYMOUTH_OVERLAY_H
#define AARAMBH_BOOT_PLYMOUTH_OVERLAY_H

#include <efi.h>

#include "aarambh/gfx/image.h"

/*
 * A small cpio archive loaded right after the OS's own initrd. Linux unpacks
 * the archives in order, so its files replace the installed Aarambh Plymouth
 * theme for this one boot: the still frame of the current theme's launch
 * splash as the background, dots in the theme's color and a script that
 * animates them where Aarambh drew them.
 */
typedef struct {
    void *data;
    UINTN size;
} PlymouthOverlay;

/*
 * Rows of a still frame that hold the splash title, as fractions of its
 * height. assets/plymouth/template/aarambh.script uses the same numbers.
 */
#define PLYMOUTH_TITLE_TOP    0.51f
#define PLYMOUTH_TITLE_HEIGHT 0.125f

int plymouth_overlay_build(PlymouthOverlay *overlay, const Image *still, UINT32 dot_color);

/* A filled dot in `color` as a PNG, for Plymouth's loading dots. */
void *plymouth_dot_png(UINT32 color, UINTN *size);
void plymouth_overlay_free(PlymouthOverlay *overlay);

#endif