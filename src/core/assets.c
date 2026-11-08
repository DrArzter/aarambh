#include "core/assets.h"

#include "aarambh/gfx/color.h"
#include "efi/fs.h"
#include "lib/str.h"

#define GRADIENT_TOP_SHADE    150
#define GRADIENT_BOTTOM_SHADE 30

static void load_font(Font *font, const char *name)
{
    font_init(font, fs_read(name, NULL));
}

static void apply_font_fallbacks(FontSet *fonts)
{
    if (!fonts->regular.loaded) {
        if (fonts->bold.loaded) {
            fonts->regular = fonts->bold;
        } else {
            fonts->regular = fonts->light;
        }
    }

    if (!fonts->light.loaded) {
        fonts->light = fonts->regular;
    }

    if (!fonts->bold.loaded) {
        fonts->bold = fonts->regular;
    }

    if (!fonts->mono.loaded) {
        fonts->mono = fonts->bold;
    }
}

void assets_load_fonts(FontSet *fonts, const Config *config)
{
    load_font(&fonts->light, config->font_light);
    load_font(&fonts->regular, config->font_regular);
    load_font(&fonts->bold, config->font_bold);
    load_font(&fonts->mono, config->font_mono);

    apply_font_fallbacks(fonts);
}

static int load_image(Image *image, const char *name)
{
    UINTN size;
    void *data;
    int decoded;

    if (str_is_empty(name)) {
        return 0;
    }

    data = fs_read(name, &size);
    if (data == NULL) {
        return 0;
    }

    decoded = image_decode(image, data, (int)size);
    mem_free(data);
    return decoded;
}

static int load_background(Entry *entry, const Image *screen)
{
    Image original;
    int covered;

    if (!load_image(&original, entry->background_name)) {
        return 0;
    }

    covered = image_cover(&entry->background, &original, screen->width, screen->height);
    image_free(&original);

    return covered;
}

static void make_fallback_background(Entry *entry, const Image *screen)
{
    UINT32 top = color_shade(entry->accent, GRADIENT_TOP_SHADE);
    UINT32 bottom = color_shade(entry->accent, GRADIENT_BOTTOM_SHADE);

    if (image_alloc(&entry->background, screen->width, screen->height)) {
        image_gradient(&entry->background, top, bottom);
    }
}

void assets_load_entries(EntryList *entries, const Image *screen)
{
    int i;

    for (i = 0; i < entries->count; i++) {
        Entry *entry = &entries->items[i];

        load_image(&entry->logo, entry->logo_name);

        if (!load_background(entry, screen)) {
            make_fallback_background(entry, screen);
        }

        image_vignette(&entry->background);
    }
}
