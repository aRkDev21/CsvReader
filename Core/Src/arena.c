#include "arena.h"
#include <string.h>

void* arena_alloc(MemoryArena* arena, size_t size) {
    size_t alligned_size = (size + 3) & ((size_t)~3);

    if (arena->offset + alligned_size > ARENA_SIZE)
        return NULL;

    void* ptr = &arena->buffer[arena->offset];
    arena->offset += alligned_size;
    return ptr;
}

char* arena_strdup(MemoryArena* arena, char* str) {
    size_t len = strlen(str) + 1;
    char* dst = (char*)arena_alloc(arena, len);
    if (dst) {
        strncpy(dst, str, len);
    }
    return dst;
}

void reset_arena(MemoryArena* arena) {
    arena->offset = 0;
}
