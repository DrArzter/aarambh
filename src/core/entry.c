#include "aarambh/entry.h"

#include "lib/str.h"

int entry_has_logo(const Entry *entry)
{
    return entry->logo.pixels != NULL;
}

int entry_has_kernels(const Entry *entry)
{
    return entry->kernel_count > 0;
}

int entry_is_system(const Entry *entry)
{
    return entry->action == ENTRY_ACTION_EFI;
}

const char *entry_action_label(const Entry *entry)
{
    switch (entry->action) {
    case ENTRY_ACTION_FIRMWARE:
        return "Open";
    case ENTRY_ACTION_REBOOT:
        return "Restart";
    case ENTRY_ACTION_SHUTDOWN:
        return "Power off";
    case ENTRY_ACTION_EFI:
        break;
    }
    return "Start";
}

void entry_subtitle(const Entry *entry, int is_default, char *output, int capacity)
{
    switch (entry->action) {
    case ENTRY_ACTION_FIRMWARE:
        str_copy(output, "Reboot into UEFI firmware setup", capacity);
        return;
    case ENTRY_ACTION_REBOOT:
        str_copy(output, "Restart this computer", capacity);
        return;
    case ENTRY_ACTION_SHUTDOWN:
        str_copy(output, "Turn off this computer", capacity);
        return;
    case ENTRY_ACTION_EFI:
        break;
    }

    if (is_default) {
        str_copy(output, "Default system", capacity);
    } else {
        str_copy(output, "System", capacity);
    }

    if (entry_has_kernels(entry)) {
        str_append(output, "  -  ", capacity);
        str_append(output, entry->kernels[0].title, capacity);
    }
}

void entry_launch_title(const Entry *entry, char *output, int capacity)
{
    switch (entry->action) {
    case ENTRY_ACTION_FIRMWARE:
        str_copy(output, "Opening ", capacity);
        str_append(output, entry->title, capacity);
        return;
    case ENTRY_ACTION_REBOOT:
        str_copy(output, "Restarting", capacity);
        return;
    case ENTRY_ACTION_SHUTDOWN:
        str_copy(output, "Shutting down", capacity);
        return;
    case ENTRY_ACTION_EFI:
        break;
    }

    str_copy(output, "Starting ", capacity);
    str_append(output, entry->title, capacity);
}

const char *entry_launch_detail(const Entry *entry, int kernel)
{
    if (!entry_is_system(entry) || !entry_has_kernels(entry)) {
        return "";
    }

    if (kernel < 0) {
        kernel = 0;
    }

    return entry->kernels[kernel].title;
}
