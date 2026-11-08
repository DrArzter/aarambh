#ifndef AARAMBH_CORE_ASSETS_H
#define AARAMBH_CORE_ASSETS_H

#include "aarambh/gfx/font.h"
#include "core/config.h"

typedef struct {
    Font light;
    Font regular;
    Font bold;
    Font mono;
} FontSet;

void assets_load_fonts(FontSet *fonts, const Config *config);
void assets_load_entries(EntryList *entries, const Image *screen);

#endif
