#ifndef AARAMBH_CORE_AUTODETECT_H
#define AARAMBH_CORE_AUTODETECT_H

#include "core/entries.h"

/*
 * Adds an entry for every boot loader under \EFI\ that no configured entry
 * uses yet: other Linux installs that never ran hooks/setup.sh, Windows,
 * systemd-boot or rEFInd.
 */
void autodetect_entries(EntryList *list);

#endif
