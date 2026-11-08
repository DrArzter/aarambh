#ifndef AARAMBH_LIB_MEMORY_H
#define AARAMBH_LIB_MEMORY_H

#include <efi.h>
#include <efilib.h>

typedef UINTN mem_size_t;

void *mem_alloc(mem_size_t size);
void *mem_realloc(void *pointer, mem_size_t size);
void mem_free(void *pointer);

void *mem_copy(void *destination, const void *source, mem_size_t size);
void *mem_set(void *destination, int value, mem_size_t size);

#endif
