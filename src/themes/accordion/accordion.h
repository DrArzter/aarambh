#ifndef AARAMBH_THEME_ACCORDION_H
#define AARAMBH_THEME_ACCORDION_H

#include "aarambh/aarambh.h"

typedef struct {
    Face title;
    Face subtitle;
    Face button;
    Face list;
    Face side;
    Face clock;
    Face clock_period;
    Face hint;
    Face letter;
    Face splash;
} AccordionFaces;

typedef struct {
    int x;
    int width;
    float weight;
} AccordionColumn;

#define ACCORDION_LIST_ROWS 6

extern AccordionFaces accordion_faces;

float accordion_launch_progress(const ThemeFrame *frame);

void accordion_layout(const ThemeContext *context, const ThemeFrame *frame,
                      AccordionColumn *columns);

void accordion_draw_side_panel(const ThemeContext *context, const Entry *entry,
                               const AccordionColumn *column, int alpha);

void accordion_draw_main_panel(const ThemeContext *context, const ThemeFrame *frame,
                               int index, const AccordionColumn *column, int alpha);

void accordion_draw_hint_bar(const ThemeContext *context, const ThemeFrame *frame,
                             const Entry *entry, float x, int alpha);

#endif
