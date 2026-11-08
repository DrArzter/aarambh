#include "themes/accordion/accordion.h"

#define HINT_BASELINE 0.935f

static void draw_list_hints(Image *screen, const UiHintStyle *face, const ThemeFrame *frame,
                            float x, int baseline, int alpha)
{
    x = ui_arrows(screen, face, x, baseline, UI_ARROWS_VERTICAL, alpha);
    x = ui_hint(screen, face, x, baseline, "Choose", alpha);
    x = ui_key_cap(screen, face, x, baseline, "Enter", alpha);
    x = ui_hint(screen, face, x, baseline, "Boot", alpha);
    x = ui_key_cap(screen, face, x, baseline, "Esc", alpha);
    x = ui_hint(screen, face, x, baseline, "Back", alpha);

    if (frame->can_edit) {
        x = ui_key_cap(screen, face, x, baseline, "E", alpha);
        ui_hint(screen, face, x, baseline, "Edit", alpha);
    }
}

static void draw_menu_hints(Image *screen, const UiHintStyle *face, const ThemeFrame *frame,
                            const Entry *entry, float x, int baseline, int alpha)
{
    x = ui_key_cap(screen, face, x, baseline, "Enter", alpha);
    x = ui_hint(screen, face, x, baseline, entry_action_label(entry), alpha);
    x = ui_arrows(screen, face, x, baseline, UI_ARROWS_HORIZONTAL, alpha);
    x = ui_hint(screen, face, x, baseline, "Switch", alpha);

    if (entry_has_kernels(entry)) {
        x = ui_arrows(screen, face, x, baseline, UI_ARROWS_VERTICAL, alpha);
        x = ui_hint(screen, face, x, baseline, "Options", alpha);
    }

    if (frame->can_edit) {
        x = ui_key_cap(screen, face, x, baseline, "E", alpha);
        ui_hint(screen, face, x, baseline, "Edit", alpha);
    }
}

void accordion_draw_hint_bar(const ThemeContext *context, const ThemeFrame *frame,
                             const Entry *entry, float x, int alpha)
{
    Image *screen = context->screen;
    UiHintStyle style = { &accordion_faces.hint, COLOR_WHITE, 1 };
    const UiHintStyle *face = &style;
    int baseline = (int)(screen->height * HINT_BASELINE);

    if (clampf(frame->list, 0.0f, 1.0f) > 0.5f) {
        draw_list_hints(screen, face, frame, x, baseline, alpha);
    } else {
        draw_menu_hints(screen, face, frame, entry, x, baseline, alpha);
    }
}