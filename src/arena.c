#include "arena.h"

#include <stdlib.h>
#include "log.h"

typedef struct ArenaBlock {
    void *ptr;
    usize used;
    usize total;
} ArenaBlock;

void arena_block_init(ArenaBlock *const self, usize const len) {
    self->ptr = malloc(len);
    self->used = 0u;
    self->total = len;
}

void arena_block_free(ArenaBlock *const self) {
    free(self->ptr);
}

// If the block can accommodate `len` more bytes, allocates that in the block and returns 
// the address. Otherwise returns NULL
void *arena_block_try_alloc(ArenaBlock *const self, usize const len) {
    if (self->used + len < self->total) {
        void *address = self->ptr + self->used;
        self->used += len;
        return address;
    } else {
        return NULL;
    }
}

ArenaBlock *arena_add_block(Arena *const self) {
    self->block_count += 1;
    self->blocks = realloc(self->blocks, self->block_count);

    ArenaBlock *const new_block = &self->blocks[self->block_count - 1u];
    arena_block_init(new_block, self->block_len);
    return new_block;
}

void arena_init(Arena *const self, usize const block_len) {
    self->blocks = NULL;
    self->block_count = 0u;
    self->block_len = block_len;
}

void arena_free(Arena *const self) {
    arena_clear(self);
}

void *arena_alloc(Arena *const self, usize const len) {
    if (len > self->block_len) {
        log_error("cannot allocate %zu bytes in arena: len > block_len", len);
        exit(1);
    }

    for (usize i = 0u; i < self->block_count; i += 1) {
        ArenaBlock *const block = &self->blocks[i];
        void *address = arena_block_try_alloc(block, len);

        if (address != NULL) {
            return address;
        }
    }

    // Arena full
    ArenaBlock *const new_block = arena_add_block(self);
    return arena_block_try_alloc(new_block, len);
}

void arena_clear(Arena *const self) {
    for (usize i = 0u; i < self->block_count; i += 1) {
        arena_block_free(&self->blocks[i]);
    }

    free(self->blocks);
    self->blocks = NULL;
}

