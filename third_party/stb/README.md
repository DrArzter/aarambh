# stb

Single-file public domain libraries by Sean Barrett: <https://github.com/nothings/stb>

| File             | Used for                         |
|------------------|----------------------------------|
| `stb_image.h`    | PNG / JPEG / BMP decoding        |
| `stb_truetype.h` | TrueType font rasterization      |

`stb_config.h` maps the libraries onto Aarambh's freestanding memory, string
and math helpers, and `stb_impl.c` compiles the implementations once.
