#include "efi/system.h"

static EFI_HANDLE image_handle;
static EFI_LOADED_IMAGE *loaded_image;

EFI_STATUS system_init(EFI_HANDLE image)
{
    EFI_STATUS status;

    image_handle = image;

    BS->SetWatchdogTimer(0, 0, 0, NULL);
    system_hide_cursor();

    status = BS->HandleProtocol(image, &LoadedImageProtocol, (void **)&loaded_image);
    if (status != EFI_SUCCESS) {
        return EFI_LOAD_ERROR;
    }

    return EFI_SUCCESS;
}

EFI_HANDLE system_image_handle(void)
{
    return image_handle;
}

EFI_HANDLE system_device_handle(void)
{
    return loaded_image->DeviceHandle;
}

void system_hide_cursor(void)
{
    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
}
