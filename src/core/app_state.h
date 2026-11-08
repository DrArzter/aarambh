#ifndef AARAMBH_CORE_APP_STATE_H
#define AARAMBH_CORE_APP_STATE_H

#include "aarambh/theme.h"
#include "core/assets.h"
#include "core/clock.h"
#include "core/config.h"
#include "core/entries.h"
#include "core/tween.h"

#define APP_MESSAGE_MAX 96
#define APP_EDIT_MAX    ((int)sizeof(((Kernel *)0)->cmdline))

typedef struct {
    Image screen;
    Config config;
    EntryList entries;
    FontSet fonts;
    Clock clock;
    const Theme *theme;
    ThemeContext context;

    int selected;
    Tween position;

    int focused;
    Tween focus;

    int list_open;
    Tween list;
    int list_selected;
    float list_scroll;

    float intro_start;

    int launching;
    float launch_start;
    int launch_kernel;

    int edit_open;
    Tween edit;
    int edit_kernel;
    int edit_applied;
    int edit_length;
    int edit_cursor;
    char edit_text[APP_EDIT_MAX];

    int countdown_active;
    float countdown_deadline;

    char message[APP_MESSAGE_MAX];
    float message_time;
    char status[APP_MESSAGE_MAX];
} AppState;

#endif