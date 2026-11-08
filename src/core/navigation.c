#include "core/navigation.h"

#include "aarambh/timing.h"
#include "lib/str.h"

Entry *nav_selected_entry(AppState *app)
{
    return &app->entries.items[app->selected];
}

void nav_select(AppState *app, int index, float now)
{
    if (index < 0 || index >= app->entries.count) {
        return;
    }

    if (index == app->selected || app->launching) {
        return;
    }

    app->selected = index;
    tween_to(&app->position, (float)index, now);

    nav_set_focus(app, 0, now);
    nav_close_list(app, now);
}

void nav_set_focus(AppState *app, int focused, float now)
{
    app->focused = focused;
    tween_to(&app->focus, (float)focused, now);
}

void nav_open_list(AppState *app, float now)
{
    app->list_open = 1;
    app->list_selected = 0;
    app->list_scroll = 0;
    tween_to(&app->list, 1.0f, now);
}

void nav_close_list(AppState *app, float now)
{
    if (!app->list_open) {
        return;
    }

    app->list_open = 0;
    tween_to(&app->list, 0.0f, now);
}

static void keep_selection_visible(AppState *app)
{
    int rows = app->theme->list_rows;
    float lowest_scroll = (float)(app->list_selected - rows + 1);

    if (app->list_selected < app->list_scroll) {
        app->list_scroll = (float)app->list_selected;
    }

    if (lowest_scroll > app->list_scroll) {
        app->list_scroll = lowest_scroll;
    }
}

void nav_move_list(AppState *app, int delta)
{
    int target = app->list_selected + delta;
    int last = nav_selected_entry(app)->kernel_count - 1;

    if (target < 0 || target > last) {
        return;
    }

    app->list_selected = target;
    keep_selection_visible(app);
}

void nav_launch(AppState *app, int kernel, float now)
{
    if (app->launching) {
        return;
    }

    app->launching = 1;
    app->launch_start = now;
    app->launch_kernel = kernel;
    app->countdown_active = 0;
}

void nav_cancel_countdown(AppState *app)
{
    app->countdown_active = 0;
}

void nav_show_message(AppState *app, const char *message, float now)
{
    str_copy(app->message, message, sizeof(app->message));
    app->message_time = now;
}

void nav_reset_after_failure(AppState *app, float now)
{
    app->launching = 0;
    app->list_open = 0;
    app->focused = 0;
    app->edit_open = 0;
    app->edit_applied = 0;

    tween_init(&app->list, 0.0f, AARAMBH_LIST_MS);
    tween_init(&app->focus, 0.0f, AARAMBH_FOCUS_MS);
    tween_init(&app->edit, 0.0f, AARAMBH_FOCUS_MS);

    app->intro_start = now;
}