#ifndef AARAMBH_GFX_PNG_H
#define AARAMBH_GFX_PNG_H

#include <efi.h>

/*
 * Encode 0xAARRGGBB pixels as a PNG in a newly allocated buffer. The image
 * data is stored uncompressed, which keeps encoding instant at boot; the file
 * only lives in RAM for the initramfs. With `with_alpha` 0 the alpha byte is
 * ignored and an RGB image is written. Returns NULL when out of memory.
 */
void *png_encode(const UINT32 *pixels, int width, int height, int with_alpha, UINTN *size);

#endif