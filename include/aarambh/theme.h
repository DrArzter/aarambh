#ifndef AARAMBH_THEME_H
#define AARAMBH_THEME_H

#include "aarambh/entry.h"
#include "aarambh/gfx/font.h"
#include "aarambh/gfx/image.h"

/*
 * Theme API
 *
 * The core owns everything that is not drawing: config, entries, kernels,
 * keyboard navigation, the countdown, animation state and booting. A theme
 * only receives a read-only snapshot of that state once per frame and paints
 * the screen.
 *
 * Adding a theme:
 *   1. Create src/themes/<name>/ and define a `const Theme <name>_theme`.
 *   2. Register it in src/core/theme_registry.c.
 *   3. Select it with `theme <name>` in aarambh.conf.
 *
 * Reusable building blocks live in aarambh/widgets.h (buttons, key hints,
 * clock, option list, launch splash) and aarambh/gfx/ (images, shapes, text).
 */

typedef struct {
    Image *screen;
    Entry *entries;
    int entry_count;
    int default_index;
    Font *font_light;
    Font *font_regular;
    Font *font_bold;
    Font *font_mono;
} ThemeContext;

typedef struct {
    float now;

    /* Milliseconds since the intro animation (re)started. */
    float intro_elapsed;

    /* Milliseconds since a launch began; only meaningful while launching. */
    float launch_elapsed;

    /* Selected entry index, animated: 1.4 means 40% of the way from 1 to 2. */
    float position;

    /* 0..1: focus moving from the primary button to "Advanced options". */
    float focus;

    /* 0..1: the advanced options list opening. */
    float list;

    int selected;
    int launching;

    /* Kernel picked from the list, or -1 for the entry's default. */
    int launch_kernel;

    /*
     * 1 when the core renders the still frame handed over to Plymouth: the
     * finished launch splash without its loading dots, which Plymouth
     * animates on top of it.
     */
    int plymouth_still;

    /*
     * With plymouth_still: NULL keeps the normal "Starting ..." title,
     * otherwise this title ("" for none) replaces it and the kernel line is
     * left out. Used for the restart and shutdown splashes.
     */
    const char *splash_title;

    /* 1 when E opens the boot options editor for what is selected now. */
    int can_edit;

    int list_selected;
    float list_scroll;

    /* Countdown or error message for the selected entry, "" when idle. */
    const char *status;

    const char *clock_time;
    const char *clock_period;
} ThemeFrame;

/* How the boot options editor is framed: a rounded card, HUD corner brackets or ink rules. */
typedef enum {
    EDIT_FRAME_CARD,
    EDIT_FRAME_BRACKETS,
    EDIT_FRAME_RULES
} EditFrame;

/*
 * Colors of the boot options editor, which the core draws on top of the
 * theme. `accent` may be UI_ENTRY_ACCENT for the selected entry's own color.
 */
typedef struct {
    UINT32 panel;
    UINT32 field;
    UINT32 accent;
    UINT32 text;
    UINT32 muted;
    EditFrame frame;
} EditColors;

typedef struct {
    const char *name;

    /* How many rows of the advanced options list fit on screen. */
    int list_rows;

    /*
     * 0: Left/Right switch entries, Down focuses "Advanced options".
     * 1: Up/Down switch entries, Right opens the kernel list (for vertical lists).
     */
    int vertical_navigation;

    /* 1: Down opens the kernel list directly (themes without an options button). */
    int down_opens_list;

    /* Colors of the boot options editor; NULL uses a neutral dark card. */
    const EditColors *edit;

    /* Called once after fonts are loaded; create Faces here. */
    void (*load)(const ThemeContext *context);

    /* Called every frame; must paint the whole screen. */
    void (*render)(const ThemeContext *context, const ThemeFrame *frame);
} Theme;

#endif