#include <efi.h>
#include <efilib.h>

#include "efi/video.h"

#define PREFERRED_MIN_WIDTH 1280
#define MAX_MODE_WIDTH 3840

static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
static UINT32 active_mode;

static UINT32 mode_area(const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info)
{
    return info->HorizontalResolution * info->VerticalResolution;
}

static void select_best_mode(void)
{
    UINT32 best_mode = gop->Mode->Mode;
    UINT32 best_area = mode_area(gop->Mode->Info);
    UINT32 mode;

    for (mode = 0; mode < gop->Mode->MaxMode; mode++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
        UINTN info_size;
        UINT32 area;

        if (gop->QueryMode(gop, mode, &info_size, &info) != EFI_SUCCESS) {
            continue;
        }

        area = mode_area(info);
        if (area > best_area && info->HorizontalResolution <= MAX_MODE_WIDTH) {
            best_mode = mode;
            best_area = area;
        }
    }

    if (best_mode != gop->Mode->Mode) {
        gop->SetMode(gop, best_mode);
    }
}

EFI_STATUS video_init(Image *screen)
{
    if (LibLocateProtocol(&GraphicsOutputProtocol, (void **)&gop) != EFI_SUCCESS) {
        return EFI_UNSUPPORTED;
    }

    if (gop->Mode->Info->HorizontalResolution < PREFERRED_MIN_WIDTH) {
        select_best_mode();
    }

    active_mode = gop->Mode->Mode;

    if (!image_alloc(screen, gop->Mode->Info->HorizontalResolution,
                     gop->Mode->Info->VerticalResolution)) {
        return EFI_OUT_OF_RESOURCES;
    }

    return EFI_SUCCESS;
}

void video_present(const Image *screen)
{
    gop->Blt(gop, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)screen->pixels, EfiBltBufferToVideo,
             0, 0, 0, 0, screen->width, screen->height, 0);
}

void video_clear(Image *screen)
{
    image_clear(screen);
    video_present(screen);
}

void video_restore_mode(void)
{
    if (gop->Mode->Mode != active_mode) {
        gop->SetMode(gop, active_mode);
    }
}
