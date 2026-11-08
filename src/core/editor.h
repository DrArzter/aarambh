#ifndef AARAMBH_CORE_EDITOR_H
#define AARAMBH_CORE_EDITOR_H

#include <efi.h>
#include <efilib.h>

#include "core/app_state.h"

/*
 * The boot options editor: E opens the kernel command line of the selected
 * entry (or of the kernel picked in the list) for one boot, like GRUB's e.
 * Only kernels Aarambh starts itself can be edited, and `edit no` in
 * aarambh.conf turns it off.
 */

void editor_load(const ThemeContext *context);

int editor_can_open(const AppState *app);
void editor_open(AppState *app, float now);

/* Handles a key while the editor is open. */
void editor_handle_key(AppState *app, const EFI_INPUT_KEY *key, float now);

/* The edited command line once Enter was pressed, otherwise NULL. */
const char *editor_cmdline(const AppState *app);

void editor_draw(AppState *app, float now);

#endif