#include <efi.h>
#include <efilib.h>

#include "boot/power.h"

#define OS_INDICATION_BOOT_TO_FW_UI 0x0000000000000001ULL

int power_firmware_setup_supported(void)
{
    UINT8 *supported = LibGetVariable(L"OsIndicationsSupported", &EfiGlobalVariable);
    int available;

    if (supported == NULL) {
        return 0;
    }

    available = (supported[0] & OS_INDICATION_BOOT_TO_FW_UI) != 0;
    FreePool(supported);
    return available;
}

void power_reboot_to_firmware(void)
{
    UINT64 indications = 0;
    UINTN size = sizeof(indications);
    UINT32 attributes = 0;
    UINT32 new_attributes = EFI_VARIABLE_NON_VOLATILE
                          | EFI_VARIABLE_BOOTSERVICE_ACCESS
                          | EFI_VARIABLE_RUNTIME_ACCESS;

    RT->GetVariable(L"OsIndications", &EfiGlobalVariable, &attributes, &size, &indications);
    indications |= OS_INDICATION_BOOT_TO_FW_UI;

    RT->SetVariable(L"OsIndications", &EfiGlobalVariable, new_attributes,
                    sizeof(indications), &indications);

    power_reboot();
}

void power_reboot(void)
{
    RT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
}

void power_shutdown(void)
{
    RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}
