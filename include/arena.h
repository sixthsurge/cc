#pragma once

#include "common.h"

struct ArenaBlock;

typedef struct Arena {
    struct ArenaBlock *blocks;
    usize block_count;
    usize block_len;
} Arena;

// Generic vec using an arena for backing storage
struct ArenaVec {
    struct Arena *backing_arena;
    void *data;
    usize element_size;
    usize len;
    usize cap;
};

void arena_init(Arena *self, usize block_len);
void arena_free(Arena *self);
void *arena_alloc(Arena *self, usize len);
void *arena_copy(Arena *self, void const *item, usize item_size);
void arena_clear(Arena *self);

void arenavec_init(struct ArenaVec *self, struct Arena *backing_arena, usize element_size);
void *arenavec_at(struct ArenaVec const *self, usize index);
void arenavec_push(struct ArenaVec *self, void *item);
