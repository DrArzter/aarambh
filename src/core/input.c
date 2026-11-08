#include <efi.h>
#include <efilib.h>

#include "core/input.h"

#include "core/editor.h"
#include "core/navigation.h"

static int is_confirm_key(const EFI_INPUT_KEY *key)
{
    return key->UnicodeChar == CHAR_CARRIAGE_RETURN || key->UnicodeChar == ' ';
}

static int is_back_key(const EFI_INPUT_KEY *key)
{
    return key->ScanCode == SCAN_ESC
        || key->ScanCode == SCAN_LEFT
        || key->UnicodeChar == CHAR_BACKSPACE;
}

static int is_edit_key(const EFI_INPUT_KEY *key)
{
    return key->UnicodeChar == 'e' || key->UnicodeChar == 'E';
}

static int is_number_key(const EFI_INPUT_KEY *key)
{
    return key->UnicodeChar >= '1' && key->UnicodeChar <= '9';
}

static void handle_list_key(AppState *app, const EFI_INPUT_KEY *key, float now)
{
    if (is_edit_key(key)) {
        if (editor_can_open(app)) {
            editor_open(app, now);
        }
    } else if (key->ScanCode == SCAN_UP) {
        nav_move_list(app, -1);
    } else if (key->ScanCode == SCAN_DOWN) {
        nav_move_list(app, 1);
    } else if (is_back_key(key)) {
        nav_close_list(app, now);
    } else if (is_confirm_key(key)) {
        nav_launch(app, app->list_selected, now);
    }
}

static int handle_vertical_key(AppState *app, const EFI_INPUT_KEY *key, float now)
{
    Entry *entry = nav_selected_entry(app);

    if (key->ScanCode == SCAN_UP) {
        nav_select(app, app->selected - 1, now);
    } else if (key->ScanCode == SCAN_DOWN) {
        nav_select(app, app->selected + 1, now);
    } else if (key->ScanCode == SCAN_RIGHT || key->UnicodeChar == CHAR_TAB) {
        if (entry_has_kernels(entry)) {
            nav_open_list(app, now);
        }
    } else {
        return 0;
    }

    return 1;
}

static void handle_menu_key(AppState *app, const EFI_INPUT_KEY *key, float now)
{
    Entry *entry = nav_selected_entry(app);

    if (is_edit_key(key)) {
        if (editor_can_open(app)) {
            editor_open(app, now);
        }
        return;
    }

    if (app->theme->vertical_navigation && handle_vertical_key(app, key, now)) {
        return;
    }

    if (key->ScanCode == SCAN_LEFT) {
        nav_select(app, app->selected - 1, now);
    } else if (key->ScanCode == SCAN_RIGHT) {
        nav_select(app, app->selected + 1, now);
    } else if (key->ScanCode == SCAN_HOME) {
        nav_select(app, 0, now);
    } else if (key->ScanCode == SCAN_END) {
        nav_select(app, app->entries.count - 1, now);
    } else if (key->ScanCode == SCAN_DOWN && entry_has_kernels(entry)
               && app->theme->down_opens_list) {
        nav_open_list(app, now);
    } else if (key->ScanCode == SCAN_DOWN && entry_has_kernels(entry) && !app->focused) {
        nav_set_focus(app, 1, now);
    } else if (key->ScanCode == SCAN_UP && app->focused) {
        nav_set_focus(app, 0, now);
    } else if (is_confirm_key(key) && app->focused) {
        nav_open_list(app, now);
    } else if (is_confirm_key(key)) {
        nav_launch(app, -1, now);
    } else if (is_number_key(key)) {
        nav_select(app, key->UnicodeChar - '1', now);
    }
}

void input_poll(AppState *app, float now)
{
    EFI_INPUT_KEY key;

    while (ST->ConIn->ReadKeyStroke(ST->ConIn, &key) == EFI_SUCCESS) {
        nav_cancel_countdown(app);

        if (app->launching) {
            continue;
        }

        if (app->edit_open) {
            editor_handle_key(app, &key, now);
        } else if (app->list_open) {
            handle_list_key(app, &key, now);
        } else {
            handle_menu_key(app, &key, now);
        }
    }
}