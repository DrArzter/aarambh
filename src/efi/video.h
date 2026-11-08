#ifndef AARAMBH_EFI_VIDEO_H
#define AARAMBH_EFI_VIDEO_H

#include "aarambh/gfx/image.h"

EFI_STATUS video_init(Image *screen);
void video_present(const Image *screen);
void video_clear(Image *screen);
void video_restore_mode(void);

#endif
