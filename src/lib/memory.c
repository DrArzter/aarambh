#include "lib/memory.h"

typedef struct {
    UINTN size;
    UINTN padding;
} BlockHeader;

void *mem_alloc(mem_size_t size)
{
    BlockHeader *header = NULL;
    EFI_STATUS status;

    status = BS->AllocatePool(EfiLoaderData, size + sizeof(BlockHeader), (void **)&header);
    if (status != EFI_SUCCESS) {
        return NULL;
    }

    header->size = size;
    return header + 1;
}

void mem_free(void *pointer)
{
    if (pointer != NULL) {
        BS->FreePool((BlockHeader *)pointer - 1);
    }
}

void *mem_realloc(void *pointer, mem_size_t size)
{
    BlockHeader *header;
    void *resized;

    if (pointer == NULL) {
        return mem_alloc(size);
    }

    header = (BlockHeader *)pointer - 1;
    if (size <= header->size) {
        return pointer;
    }

    resized = mem_alloc(size);
    if (resized == NULL) {
        return NULL;
    }

    mem_copy(resized, pointer, header->size);
    mem_free(pointer);
    return resized;
}

void *mem_copy(void *destination, const void *source, mem_size_t size)
{
    UINT8 *target = destination;
    const UINT8 *origin = source;

    while (size >= 8) {
        *(UINT64 *)target = *(const UINT64 *)origin;
        target += 8;
        origin += 8;
        size -= 8;
    }

    while (size > 0) {
        *target = *origin;
        target++;
        origin++;
        size--;
    }

    return destination;
}

void *mem_set(void *destination, int value, mem_size_t size)
{
    UINT8 *target = destination;

    while (size > 0) {
        *target = (UINT8)value;
        target++;
        size--;
    }

    return destination;
}
