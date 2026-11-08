#include "boot/plymouth_overlay.h"

#include "gfx/png.h"
#include "lib/str.h"

#define THEME_DIR     "usr/share/plymouth/themes/aarambh"
#define MODE_DIR      0040755
#define MODE_FILE     0100644
#define CPIO_HEADER   110
#define DOT_SIZE      64

typedef struct {
    UINT8 *out;
    UINTN size;
    UINT32 inode;
} Cpio;

/* Same layout as ui_launch_splash_draw and ui_loading_dots. */
static const char splash_script[] =
    "W = Window.GetWidth();\n"
    "H = Window.GetHeight();\n"
    "Window.SetBackgroundTopColor(0, 0, 0);\n"
    "Window.SetBackgroundBottomColor(0, 0, 0);\n"
    "\n"
    "bgi = Image(\"bg.png\");\n"
    "s = W / bgi.GetWidth();\n"
    "if (H / bgi.GetHeight() > s)\n"
    "  s = H / bgi.GetHeight();\n"
    "bgi = bgi.Scale(Math.Int(bgi.GetWidth() * s + 1), Math.Int(bgi.GetHeight() * s + 1));\n"
    "ox = Math.Int((W - bgi.GetWidth()) / 2);\n"
    "oy = Math.Int((H - bgi.GetHeight()) / 2);\n"
    "bg = Sprite(bgi);\n"
    "bg.SetPosition(ox, oy, -100);\n"
    "\n"
    "u = bgi.GetHeight();\n"
    "r = u * 0.0068;\n"
    "dot = Image(\"dot.png\").Scale(Math.Int(r * 2), Math.Int(r * 2));\n"
    "for (i = 0; i < 3; i++) {\n"
    "  dots[i] = Sprite(dot);\n"
    "  dots[i].SetPosition(Math.Int(ox + bgi.GetWidth() / 2 + (i - 1) * u * 0.028 - r),"
    " Math.Int(oy + u * 0.725 - r), 4);\n"
    "}\n"
    "\n"
    "frame = 0;\n"
    "fun refresh() {\n"
    "  global.frame++;\n"
    "  step = Math.Int(global.frame / 13);\n"
    "  active = step - Math.Int(step / 3) * 3;\n"
    "  for (i = 0; i < 3; i++) {\n"
    "    if (i == active)\n"
    "      global.dots[i].SetOpacity(1);\n"
    "    else\n"
    "      global.dots[i].SetOpacity(0.43);\n"
    "  }\n"
    "}\n"
    "Plymouth.SetRefreshFunction(refresh);\n"
    "\n"
    "pimg = Image(\"prompt.png\");\n"
    "k = H / 1080;\n"
    "pimg = pimg.Scale(Math.Int(pimg.GetWidth() * k), Math.Int(pimg.GetHeight() * k));\n"
    "bsz = Math.Int(H * 0.012) + 1;\n"
    "bimg = Image(\"dot.png\").Scale(bsz, bsz);\n"
    "\n"
    "fun display_password(prompt, bullets) {\n"
    "  global.ps = Sprite(global.pimg);\n"
    "  global.ps.SetPosition(Math.Int((global.W - global.pimg.GetWidth()) / 2),"
    " Math.Int(global.H * 0.8), 10);\n"
    "  n = bullets;\n"
    "  if (n > 30)\n"
    "    n = 30;\n"
    "  gap = global.bsz * 1.8;\n"
    "  x0 = global.W / 2 - (n * gap) / 2;\n"
    "  for (i = 0; i < 30; i++) {\n"
    "    if (i < n) {\n"
    "      global.bs[i] = Sprite(global.bimg);\n"
    "      global.bs[i].SetPosition(Math.Int(x0 + i * gap), Math.Int(global.H * 0.865), 10);\n"
    "    } else {\n"
    "      global.bs[i] = NULL;\n"
    "    }\n"
    "  }\n"
    "}\n"
    "\n"
    "fun display_normal() {\n"
    "  global.ps = NULL;\n"
    "  for (i = 0; i < 30; i++)\n"
    "    global.bs[i] = NULL;\n"
    "}\n"
    "\n"
    "Plymouth.SetDisplayPasswordFunction(display_password);\n"
    "Plymouth.SetDisplayNormalFunction(display_normal);\n";

static UINTN align4(UINTN value)
{
    return (value + 3) & ~(UINTN)3;
}

static void put_hex(UINT8 *out, UINT32 value)
{
    static const char digits[] = "0123456789abcdef";
    int i;

    for (i = 7; i >= 0; i--) {
        out[i] = (UINT8)digits[value & 0xf];
        value >>= 4;
    }
}

/* One newc record. With cpio->out NULL it only counts the bytes. */
static void cpio_add(Cpio *cpio, const char *name, UINT32 mode, const void *data, UINTN size)
{
    UINT32 fields[13];
    UINTN name_size = str_length(name) + 1;
    UINTN start = cpio->size;
    int i;

    cpio->size = align4(start + CPIO_HEADER + name_size);
    cpio->size = align4(cpio->size + size);

    if (cpio->out == NULL) {
        return;
    }

    mem_set(fields, 0, sizeof(fields));
    fields[0] = ++cpio->inode;
    fields[1] = mode;
    fields[4] = (mode & 0040000) ? 2 : 1;
    fields[6] = (UINT32)size;
    fields[11] = (UINT32)name_size;

    mem_set(cpio->out + start, 0, cpio->size - start);
    mem_copy(cpio->out + start, "070701", 6);
    for (i = 0; i < 13; i++) {
        put_hex(cpio->out + start + 6 + i * 8, fields[i]);
    }

    mem_copy(cpio->out + start + CPIO_HEADER, name, name_size);
    if (size > 0) {
        mem_copy(cpio->out + align4(start + CPIO_HEADER + name_size), data, size);
    }
}

static void cpio_write_all(Cpio *cpio, const void *bg, UINTN bg_size, const void *dot,
                           UINTN dot_size)
{
    cpio_add(cpio, "usr", MODE_DIR, NULL, 0);
    cpio_add(cpio, "usr/share", MODE_DIR, NULL, 0);
    cpio_add(cpio, "usr/share/plymouth", MODE_DIR, NULL, 0);
    cpio_add(cpio, "usr/share/plymouth/themes", MODE_DIR, NULL, 0);
    cpio_add(cpio, THEME_DIR, MODE_DIR, NULL, 0);
    cpio_add(cpio, THEME_DIR "/bg.png", MODE_FILE, bg, bg_size);
    cpio_add(cpio, THEME_DIR "/dot.png", MODE_FILE, dot, dot_size);
    cpio_add(cpio, THEME_DIR "/aarambh.script", MODE_FILE, splash_script,
             sizeof(splash_script) - 1);
    cpio_add(cpio, "TRAILER!!!", 0, NULL, 0);
}

void *plymouth_dot_png(UINT32 color, UINTN *size)
{
    static UINT32 pixels[DOT_SIZE * DOT_SIZE];
    float center = (DOT_SIZE - 1) / 2.0f;
    float radius = DOT_SIZE / 2.0f;
    int x;
    int y;

    for (y = 0; y < DOT_SIZE; y++) {
        for (x = 0; x < DOT_SIZE; x++) {
            float dx = x - center;
            float dy = y - center;
            float edge = radius - 0.5f;
            float distance_squared = dx * dx + dy * dy;
            UINT32 alpha = 0;

            if (distance_squared <= (edge - 0.5f) * (edge - 0.5f)) {
                alpha = 255;
            } else if (distance_squared <= (edge + 0.5f) * (edge + 0.5f)) {
                alpha = 128;
            }

            pixels[y * DOT_SIZE + x] = (alpha << 24) | (color & 0xffffff);
        }
    }

    return png_encode(pixels, DOT_SIZE, DOT_SIZE, 1, size);
}

int plymouth_overlay_build(PlymouthOverlay *overlay, const Image *still, UINT32 dot_color)
{
    Cpio cpio;
    UINTN bg_size;
    UINTN dot_size;
    void *bg;
    void *dot;

    overlay->data = NULL;
    overlay->size = 0;

    bg = png_encode(still->pixels, still->width, still->height, 0, &bg_size);
    dot = plymouth_dot_png(dot_color, &dot_size);

    if (bg != NULL && dot != NULL) {
        mem_set(&cpio, 0, sizeof(cpio));
        cpio_write_all(&cpio, bg, bg_size, dot, dot_size);

        overlay->size = cpio.size;
        cpio.out = mem_alloc(cpio.size);
        cpio.size = 0;

        if (cpio.out != NULL) {
            cpio_write_all(&cpio, bg, bg_size, dot, dot_size);
            overlay->data = cpio.out;
        } else {
            overlay->size = 0;
        }
    }

    mem_free(bg);
    mem_free(dot);
    return overlay->data != NULL;
}

void plymouth_overlay_free(PlymouthOverlay *overlay)
{
    mem_free(overlay->data);
    overlay->data = NULL;
    overlay->size = 0;
}