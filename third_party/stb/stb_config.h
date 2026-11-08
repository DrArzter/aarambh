#ifndef AARAMBH_STB_CONFIG_H
#define AARAMBH_STB_CONFIG_H

#include "lib/math.h"
#include "lib/memory.h"
#include "lib/str.h"

#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ASSERT(x) ((void)0)
#define STBI_MALLOC(size) mem_alloc(size)
#define STBI_REALLOC(pointer, size) mem_realloc(pointer, size)
#define STBI_FREE(pointer) mem_free(pointer)

#define STBTT_ifloor(x) ((int)math_floor(x))
#define STBTT_iceil(x) ((int)math_ceil(x))
#define STBTT_sqrt(x) math_sqrt(x)
#define STBTT_pow(x, y) math_pow(x, y)
#define STBTT_fmod(x, y) math_fmod(x, y)
#define STBTT_cos(x) math_cos(x)
#define STBTT_acos(x) math_acos(x)
#define STBTT_fabs(x) math_fabs(x)
#define STBTT_malloc(size, user) ((void)(user), mem_alloc(size))
#define STBTT_free(pointer, user) ((void)(user), mem_free(pointer))
#define STBTT_assert(x) ((void)0)
#define STBTT_strlen(text) str_length(text)
#define STBTT_memcpy mem_copy
#define STBTT_memset mem_set

#endif
