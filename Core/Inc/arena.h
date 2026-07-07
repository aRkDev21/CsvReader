#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ARENA_SIZE (1024 * 4)

typedef struct {
    __attribute__((aligned(4)))uint8_t buffer[ARENA_SIZE];
    size_t offset;


} MemoryArena;

void* arena_alloc(MemoryArena* arena, size_t size);
char* arena_strdup(MemoryArena* arena, char* str);
void reset_arena(MemoryArena* arena);

#endif
