#include "themes/accordion/accordion.h"

#define EXPANDED_SHARE      0.52f
#define MAX_COLLAPSED_SHARE 0.16f

typedef struct {
    float collapsed;
    float expanded;
} ColumnSizes;

static ColumnSizes base_sizes(float width, int count)
{
    ColumnSizes sizes;

    if (count <= 1) {
        sizes.expanded = width;
        sizes.collapsed = 0.0f;
        return sizes;
    }

    sizes.expanded = width * EXPANDED_SHARE;
    sizes.collapsed = (width - sizes.expanded) / (count - 1);

    if (sizes.collapsed > width * MAX_COLLAPSED_SHARE) {
        sizes.collapsed = width * MAX_COLLAPSED_SHARE;
        sizes.expanded = width - sizes.collapsed * (count - 1);
    }

    return sizes;
}

static float column_weight(const ThemeFrame *frame, int index)
{
    if (frame->launching) {
        if (index == frame->selected) {
            return 1.0f;
        }
        return 0.0f;
    }

    return clampf(1.0f - absf(index - frame->position), 0.0f, 1.0f);
}

void accordion_layout(const ThemeContext *context, const ThemeFrame *frame,
                      AccordionColumn *columns)
{
    float width = (float)context->screen->width;
    int count = context->entry_count;
    ColumnSizes sizes = base_sizes(width, count);
    float shrink = 1.0f - accordion_launch_progress(frame);
    float collapsed = sizes.collapsed * shrink;
    float expanded = width - collapsed * (count - 1);
    float cursor = 0.0f;
    int i;

    for (i = 0; i < count; i++) {
        float weight = column_weight(frame, i);
        float column_width = collapsed + (expanded - collapsed) * weight;

        columns[i].x = (int)(cursor + 0.5f);
        columns[i].width = (int)(cursor + column_width + 0.5f) - columns[i].x;
        columns[i].weight = weight;

        cursor += column_width;
    }
}
