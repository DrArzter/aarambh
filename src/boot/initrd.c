#include <efi.h>
#include <efilib.h>

#include "boot/initrd.h"

#include "lib/memory.h"

typedef struct InitrdLoadFile2 {
    EFI_STATUS (EFIAPI *LoadFile)(struct InitrdLoadFile2 *self, EFI_DEVICE_PATH *path,
                                  BOOLEAN boot_policy, UINTN *size, VOID *buffer);
} InitrdLoadFile2;

typedef struct {
    VENDOR_DEVICE_PATH vendor;
    EFI_DEVICE_PATH end;
} __attribute__((packed)) InitrdDevicePath;

static EFI_GUID load_file2_guid = {
    0x4006c0c1, 0xfcb3, 0x403e, { 0x99, 0x6d, 0x4a, 0x6c, 0x87, 0x24, 0xe0, 0x6d }
};

static EFI_GUID linux_initrd_media_guid = {
    0x5568e427, 0x68fc, 0x4f3d, { 0xac, 0x74, 0xca, 0x55, 0x52, 0x31, 0xcc, 0x68 }
};

static void *initrd_data;
static UINTN initrd_size;
static EFI_HANDLE initrd_handle;
static InitrdDevicePath initrd_path;

static EFI_STATUS EFIAPI initrd_load_file(InitrdLoadFile2 *self, EFI_DEVICE_PATH *path,
                                          BOOLEAN boot_policy, UINTN *size, VOID *buffer)
{
    if (boot_policy) {
        return EFI_UNSUPPORTED;
    }

    if (size == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    if (buffer == NULL || *size < initrd_size) {
        *size = initrd_size;
        return EFI_BUFFER_TOO_SMALL;
    }

    mem_copy(buffer, initrd_data, initrd_size);
    *size = initrd_size;
    return EFI_SUCCESS;
}

static InitrdLoadFile2 initrd_protocol = { initrd_load_file };

static void build_device_path(void)
{
    mem_set(&initrd_path, 0, sizeof(initrd_path));

    initrd_path.vendor.Header.Type = MEDIA_DEVICE_PATH;
    initrd_path.vendor.Header.SubType = MEDIA_VENDOR_DP;
    initrd_path.vendor.Header.Length[0] = sizeof(VENDOR_DEVICE_PATH);
    initrd_path.vendor.Guid = linux_initrd_media_guid;

    initrd_path.end.Type = END_DEVICE_PATH_TYPE;
    initrd_path.end.SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;
    initrd_path.end.Length[0] = sizeof(EFI_DEVICE_PATH);
}

EFI_STATUS initrd_register(void *data, UINTN size)
{
    EFI_STATUS status;

    initrd_data = data;
    initrd_size = size;
    build_device_path();

    status = BS->InstallProtocolInterface(&initrd_handle, &DevicePathProtocol,
                                          EFI_NATIVE_INTERFACE, &initrd_path);
    if (!EFI_ERROR(status)) {
        status = BS->InstallProtocolInterface(&initrd_handle, &load_file2_guid,
                                              EFI_NATIVE_INTERFACE, &initrd_protocol);
    }

    if (EFI_ERROR(status)) {
        initrd_unregister();
    }

    return status;
}

void initrd_unregister(void)
{
    if (initrd_handle != NULL) {
        BS->UninstallProtocolInterface(initrd_handle, &load_file2_guid, &initrd_protocol);
        BS->UninstallProtocolInterface(initrd_handle, &DevicePathProtocol, &initrd_path);
        initrd_handle = NULL;
    }

    mem_free(initrd_data);
    initrd_data = NULL;
    initrd_size = 0;
}
