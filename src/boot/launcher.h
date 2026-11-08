#ifndef AARAMBH_BOOT_LAUNCHER_H
#define AARAMBH_BOOT_LAUNCHER_H

#include "aarambh/entry.h"
#include "boot/plymouth_overlay.h"
#include "efi/fs.h"

/* 1 when Aarambh starts this kernel itself, so a Plymouth overlay reaches it. */
int launcher_boots_direct(const Entry *entry, int kernel);

/*
 * `cmdline` replaces the kernel's command line for this boot, or is NULL. An
 * edited boot never falls back to the OS's own loader, which would drop the edit.
 */
EFI_STATUS launcher_run(const Entry *entry, int kernel, const char *cmdline,
                        const PlymouthOverlay *overlay, FsProgressCallback progress);

#endif