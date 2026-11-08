#include "themes/accordion/accordion.h"

#include "lib/str.h"

#define SIDE_LOGO_SHARE    0.42f
#define SIDE_LOGO_MAX      0.085f
#define SIDE_LOGO_Y        0.38f
#define SIDE_LOGO_ALPHA    225
#define SIDE_TITLE_BOTTOM  0.93f
#define SIDE_TITLE_ALPHA   215
#define SIDE_SHADOW_ALPHA  90
#define SIDE_SHADOW_OFFSET 2

static void draw_side_logo(Image *screen, const Entry *entry,
                           const AccordionColumn *column, int alpha)
{
    float height = (float)screen->height;
    int size = (int)clampf(column->width * SIDE_LOGO_SHARE, 0.0f, height * SIDE_LOGO_MAX);
    int x = column->x + (column->width - size) / 2;
    int y = (int)(height * SIDE_LOGO_Y);

    if (entry_has_logo(entry)) {
        draw_image(screen, &entry->logo, x, y, size, size, alpha * SIDE_LOGO_ALPHA / 256);
    }
}

static void draw_side_title(Image *screen, const Entry *entry,
                            const AccordionColumn *column, int alpha)
{
    Face *face = &accordion_faces.side;
    char title[64];
    int line_height = face_line_height(face);
    int x = column->x + (column->width - line_height) / 2;
    int bottom = (int)(screen->height * SIDE_TITLE_BOTTOM);

    str_to_upper(title, entry->title, sizeof(title));

    draw_text_vertical(screen, face, x + SIDE_SHADOW_OFFSET, bottom + SIDE_SHADOW_OFFSET,
                       title, COLOR_BLACK, alpha * SIDE_SHADOW_ALPHA / 256);
    draw_text_vertical(screen, face, x, bottom, title, COLOR_WHITE,
                       alpha * SIDE_TITLE_ALPHA / 256);
}

void accordion_draw_side_panel(const ThemeContext *context, const Entry *entry,
                               const AccordionColumn *column, int alpha)
{
    if (alpha <= 0) {
        return;
    }

    draw_side_logo(context->screen, entry, column, alpha);
    draw_side_title(context->screen, entry, column, alpha);
}
