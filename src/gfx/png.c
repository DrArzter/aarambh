#include "gfx/png.h"

#include "lib/memory.h"

#define DEFLATE_BLOCK_MAX 65535
#define ADLER_MOD         65521

static UINT32 crc_table[256];
static int crc_ready;

static void build_crc_table(void)
{
    UINT32 n;
    int k;

    for (n = 0; n < 256; n++) {
        UINT32 c = n;

        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320u ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }

        crc_table[n] = c;
    }

    crc_ready = 1;
}

static UINT32 crc_update(UINT32 crc, const UINT8 *data, UINTN size)
{
    UINTN i;

    for (i = 0; i < size; i++) {
        crc = crc_table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    }

    return crc;
}

static UINT8 *put_be32(UINT8 *out, UINT32 value)
{
    out[0] = (UINT8)(value >> 24);
    out[1] = (UINT8)(value >> 16);
    out[2] = (UINT8)(value >> 8);
    out[3] = (UINT8)value;
    return out + 4;
}

/* Writes length, type and data, then the CRC over type and data. */
static UINT8 *put_chunk(UINT8 *out, const char *type, const UINT8 *data, UINTN size)
{
    UINT8 *start;
    UINT32 crc;

    out = put_be32(out, (UINT32)size);
    start = out;
    mem_copy(out, type, 4);
    out += 4;
    if (size > 0) {
        mem_copy(out, data, size);
        out += size;
    }

    crc = crc_update(0xffffffffu, start, (UINTN)(out - start));
    return put_be32(out, crc ^ 0xffffffffu);
}

static void fill_scanlines(UINT8 *raw, const UINT32 *pixels, int width, int height,
                           int with_alpha)
{
    int x;
    int y;

    for (y = 0; y < height; y++) {
        *raw++ = 0;

        for (x = 0; x < width; x++) {
            UINT32 pixel = pixels[y * width + x];

            *raw++ = (UINT8)(pixel >> 16);
            *raw++ = (UINT8)(pixel >> 8);
            *raw++ = (UINT8)pixel;
            if (with_alpha) {
                *raw++ = (UINT8)(pixel >> 24);
            }
        }
    }
}

/* zlib stream of stored deflate blocks around `raw`. */
static UINT8 *put_zlib(UINT8 *out, const UINT8 *raw, UINTN size)
{
    UINT32 a = 1;
    UINT32 b = 0;
    UINTN offset = 0;
    UINTN i;

    *out++ = 0x78;
    *out++ = 0x01;

    do {
        UINTN block = size - offset;

        if (block > DEFLATE_BLOCK_MAX) {
            block = DEFLATE_BLOCK_MAX;
        }

        *out++ = (UINT8)(offset + block == size);
        *out++ = (UINT8)block;
        *out++ = (UINT8)(block >> 8);
        *out++ = (UINT8)~block;
        *out++ = (UINT8)(~block >> 8);
        mem_copy(out, raw + offset, block);
        out += block;
        offset += block;
    } while (offset < size);

    for (i = 0; i < size; i++) {
        a = (a + raw[i]) % ADLER_MOD;
        b = (b + a) % ADLER_MOD;
    }

    return put_be32(out, (b << 16) | a);
}

void *png_encode(const UINT32 *pixels, int width, int height, int with_alpha, UINTN *size)
{
    static const UINT8 signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
    int channels = with_alpha ? 4 : 3;
    UINTN raw_size = (UINTN)height * (1 + (UINTN)width * channels);
    UINTN blocks = raw_size / DEFLATE_BLOCK_MAX + 1;
    UINTN zlib_size = 2 + raw_size + blocks * 5 + 4;
    UINT8 header[13];
    UINT8 *raw;
    UINT8 *zlib;
    UINT8 *png;
    UINT8 *out;

    if (!crc_ready) {
        build_crc_table();
    }

    raw = mem_alloc(raw_size);
    zlib = mem_alloc(zlib_size);
    png = mem_alloc(8 + 25 + 12 + zlib_size + 12);
    if (raw == NULL || zlib == NULL || png == NULL) {
        mem_free(raw);
        mem_free(zlib);
        mem_free(png);
        return NULL;
    }

    fill_scanlines(raw, pixels, width, height, with_alpha);
    zlib_size = (UINTN)(put_zlib(zlib, raw, raw_size) - zlib);
    mem_free(raw);

    put_be32(header, (UINT32)width);
    put_be32(header + 4, (UINT32)height);
    header[8] = 8;
    header[9] = with_alpha ? 6 : 2;
    header[10] = 0;
    header[11] = 0;
    header[12] = 0;

    mem_copy(png, signature, sizeof(signature));
    out = png + sizeof(signature);
    out = put_chunk(out, "IHDR", header, sizeof(header));
    out = put_chunk(out, "IDAT", zlib, zlib_size);
    out = put_chunk(out, "IEND", NULL, 0);
    mem_free(zlib);

    *size = (UINTN)(out - png);
    return png;
}