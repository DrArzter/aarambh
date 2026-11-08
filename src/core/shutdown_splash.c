#include "core/shutdown_splash.h"

#include "aarambh/widgets.h"
#include "boot/plymouth_overlay.h"
#include "efi/fs.h"
#include "gfx/png.h"
#include "lib/memory.h"
#include "lib/str.h"

#define SPLASH_DIR "shutdown"
#define STAMP_MAX  96

static void file_name(char *output, int capacity, const Entry *entry, const char *suffix)
{
    str_copy(output, SPLASH_DIR "\\", capacity);
    str_append(output, entry->key, capacity);
    str_append(output, suffix, capacity);
}

static void build_stamp(char *stamp, const char *theme, const Image *screen)
{
    str_copy(stamp, theme, STAMP_MAX);
    str_append(stamp, " ", STAMP_MAX);
    str_append_int(stamp, screen->width, STAMP_MAX);
    str_append(stamp, "x", STAMP_MAX);
    str_append_int(stamp, screen->height, STAMP_MAX);
}

static int stamp_matches(const char *name, const char *stamp)
{
    char *saved = fs_read(name, NULL);
    int matches = saved != NULL && str_equal(saved, stamp);

    mem_free(saved);
    return matches;
}

static int write_png(const char *name, const UINT32 *pixels, int width, int height)
{
    UINTN size;
    void *png = png_encode(pixels, width, height, 0, &size);
    int written = png != NULL && fs_write_data(name, png, size);

    mem_free(png);
    return written;
}

static int write_title(const Entry *entry, const char *suffix, const char *title,
                       const Image *screen, StillRenderer render)
{
    char name[64];
    int top = (int)(screen->height * PLYMOUTH_TITLE_TOP);
    int rows = (int)(screen->height * PLYMOUTH_TITLE_HEIGHT);

    render(title);
    file_name(name, sizeof(name), entry, suffix);
    return write_png(name, screen->pixels + (UINTN)top * screen->width, screen->width, rows);
}

static int write_dot(const Entry *entry)
{
    char name[64];
    UINTN size;
    void *png = plymouth_dot_png(ui_launch_splash_dot_color(), &size);
    int written;

    file_name(name, sizeof(name), entry, "-dot.png");
    written = png != NULL && fs_write_data(name, png, size);
    mem_free(png);
    return written;
}

void shutdown_splash_update(const Entry *entry, const char *theme, const Image *screen,
                            StillRenderer render)
{
    char stamp_name[64];
    char bg_name[64];
    char stamp[STAMP_MAX];

    if (!entry_has_kernels(entry) || str_is_empty(entry->key)) {
        return;
    }

    build_stamp(stamp, theme, screen);
    file_name(stamp_name, sizeof(stamp_name), entry, ".stamp");
    if (stamp_matches(stamp_name, stamp)) {
        return;
    }

    fs_make_directory(SPLASH_DIR);

    render("");
    file_name(bg_name, sizeof(bg_name), entry, "-bg.png");

    if (write_png(bg_name, screen->pixels, screen->width, screen->height)
        && write_dot(entry)
        && write_title(entry, "-reboot.png", "Restarting", screen, render)
        && write_title(entry, "-shutdown.png", "Shutting down", screen, render)) {
        fs_write(stamp_name, stamp);
    }
}