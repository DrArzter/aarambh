#ifndef AARAMBH_BOOT_POWER_H
#define AARAMBH_BOOT_POWER_H

int power_firmware_setup_supported(void);

void power_reboot_to_firmware(void);
void power_reboot(void);
void power_shutdown(void);

#endif
