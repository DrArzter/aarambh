#ifndef AARAMBH_GFX_COLOR_H
#define AARAMBH_GFX_COLOR_H

#include <efi.h>

/*
 * Colors are 0xAARRGGBB. Drawing functions ignore the alpha byte of the color
 * and take a separate opacity argument in the range 0..256.
 */

#define COLOR_WHITE 0xffffff
#define COLOR_BLACK 0x000000
#define COLOR_INK   0x111111

#define ALPHA_OPAQUE 256

UINT32 color_shade(UINT32 color, int factor);

#endif
