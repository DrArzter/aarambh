#include "core/theme_registry.h"

#include "lib/str.h"

extern const Theme accordion_theme;
extern const Theme minimal_theme;
extern const Theme glass_sidebar_theme;
extern const Theme editorial_theme;
extern const Theme editorial_dark_theme;
extern const Theme cinema_theme;
extern const Theme honeycomb_theme;
extern const Theme constellation_theme;
extern const Theme picker_wheel_theme;
extern const Theme dial_theme;
extern const Theme hud_theme;

static const Theme *const themes[] = {
    &accordion_theme,
    &minimal_theme,
    &glass_sidebar_theme,
    &editorial_theme,
    &editorial_dark_theme,
    &cinema_theme,
    &honeycomb_theme,
    &constellation_theme,
    &picker_wheel_theme,
    &dial_theme,
    &hud_theme
};

const Theme *theme_registry_find(const char *name)
{
    int count = (int)(sizeof(themes) / sizeof(themes[0]));
    int i;

    for (i = 0; i < count; i++) {
        if (str_equal(themes[i]->name, name)) {
            return themes[i];
        }
    }

    return themes[0];
}