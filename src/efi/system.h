#ifndef AARAMBH_EFI_SYSTEM_H
#define AARAMBH_EFI_SYSTEM_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS system_init(EFI_HANDLE image);

EFI_HANDLE system_image_handle(void);
EFI_HANDLE system_device_handle(void);

void system_hide_cursor(void);

#endif
