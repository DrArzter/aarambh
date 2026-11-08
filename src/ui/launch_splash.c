#include "aarambh/easing.h"
#include "aarambh/gfx/color.h"
#include "aarambh/gfx/draw.h"
#include "aarambh/timing.h"
#include "aarambh/widgets.h"
#include "lib/str.h"

#define SPLASH_START       (AARAMBH_LAUNCH_MS * 0.55f)
#define SPLASH_RISE        0.03f
#define SPLASH_LOGO_SIZE   0.2f
#define SPLASH_LOGO_Y      0.3f
#define SPLASH_TITLE_Y     0.6f
#define SPLASH_DETAIL_Y    0.655f
#define SPLASH_DOTS_Y      0.725f
#define DETAIL_FADE_LEAD   420.0f
#define DETAIL_FADE_MS     300.0f

static UINT32 last_dot_color = COLOR_WHITE;

UINT32 ui_launch_splash_dot_color(void)
{
    return last_dot_color;
}

static void draw_centered(Image *screen, const UiSplash *splash, Face *face, int baseline,
                          const char *text, UINT32 color, int alpha)
{
    float x = (screen->width - text_width(face, text)) / 2;

    if (splash->shadow) {
        draw_text_shadow(screen, face, x, baseline, text, color, alpha);
    } else {
        draw_text(screen, face, x, baseline, text, color, alpha);
    }
}

static int detail_alpha(const ThemeFrame *frame, int alpha)
{
    float fade_start = AARAMBH_LAUNCH_TOTAL_MS - DETAIL_FADE_LEAD;
    float fade = clampf((frame->launch_elapsed - fade_start) / DETAIL_FADE_MS, 0.0f, 1.0f);

    return (int)(alpha * (1.0f - fade));
}

void ui_launch_splash_draw(const ThemeContext *context, const ThemeFrame *frame,
                           const UiSplash *splash)
{
    Image *screen = context->screen;
    Entry *entry = &context->entries[frame->selected];
    float width = (float)screen->width;
    float height = (float)screen->height;
    float progress;
    float rise;
    int alpha;
    char title[96];
    const char *detail;

    last_dot_color = splash->dot_color;

    progress = ease_in_out_cubic((frame->launch_elapsed - SPLASH_START) / AARAMBH_SPLASH_MS);
    if (progress <= 0.0f) {
        return;
    }

    alpha = (int)(progress * 256.0f);
    rise = (1.0f - progress) * height * SPLASH_RISE;

    if (splash->backdrop != NULL && splash->backdrop->pixels != NULL) {
        image_blend(screen, screen, splash->backdrop, alpha);
    } else {
        image_blend(screen, screen, &entry->background, alpha);
    }
    image_darken(screen, (int)(progress * splash->darken));

    if (entry_has_logo(entry)) {
        int size = (int)(height * SPLASH_LOGO_SIZE);
        int x = (int)((width - size) / 2);
        int y = (int)(height * SPLASH_LOGO_Y + rise);

        draw_image_tinted(screen, &entry->logo, x, y, size, size, splash->title_color, alpha);
    }

    entry_launch_title(entry, title, sizeof(title));
    detail = entry_launch_detail(entry, frame->launch_kernel);
    if (frame->plymouth_still && frame->splash_title != NULL) {
        str_copy(title, frame->splash_title, sizeof(title));
        detail = "";
    }

    draw_centered(screen, splash, splash->title_face, (int)(height * SPLASH_TITLE_Y + rise), title,
                  splash->title_color, alpha);

    if (detail[0] != '\0') {
        draw_centered(screen, splash, splash->detail_face,
                      (int)(height * SPLASH_DETAIL_Y + rise), detail, splash->detail_color,
                      detail_alpha(frame, alpha));
    }

    if (!frame->plymouth_still) {
        ui_loading_dots(screen, width / 2, height * SPLASH_DOTS_Y + rise, frame->now,
                        splash->dot_color, alpha);
    }
}