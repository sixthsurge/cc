#pragma once

#include "common.h"

struct ArenaBlock;

typedef struct Arena {
    struct ArenaBlock *blocks;
    usize block_count;
    usize block_len;
} Arena;

void arena_init(Arena *self, usize block_len);
void arena_free(Arena *self);
void *arena_alloc(Arena *self, usize len);
void arena_clear(Arena *self);
