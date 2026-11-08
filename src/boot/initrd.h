#ifndef AARAMBH_BOOT_INITRD_H
#define AARAMBH_BOOT_INITRD_H

#include <efi.h>

EFI_STATUS initrd_register(void *data, UINTN size);
void initrd_unregister(void);

#endif
