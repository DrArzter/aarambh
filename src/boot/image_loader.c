#include <efi.h>
#include <efilib.h>

#include "boot/image_loader.h"

#include "efi/system.h"

EFI_STATUS image_loader_load(const CHAR16 *path, EFI_HANDLE *child)
{
    EFI_DEVICE_PATH *device_path;
    EFI_STATUS status;

    *child = NULL;

    device_path = FileDevicePath(system_device_handle(), (CHAR16 *)path);
    if (device_path == NULL) {
        return EFI_NOT_FOUND;
    }

    status = BS->LoadImage(FALSE, system_image_handle(), device_path, NULL, 0, child);
    FreePool(device_path);

    return status;
}

EFI_STATUS image_loader_start(const CHAR16 *path)
{
    EFI_HANDLE child;
    EFI_STATUS status;

    status = image_loader_load(path, &child);
    if (EFI_ERROR(status)) {
        return status;
    }

    return BS->StartImage(child, NULL, NULL);
}
