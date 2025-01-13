#include "arena.h"

#include <stdlib.h>
#include <string.h>
#include "log.h"

struct ArenaBlock {
    void *ptr;
    usize used;
    usize total;
};

static void arena_block_init(struct ArenaBlock *const self, usize const len) {
    self->ptr = malloc(len);
    self->used = 0u;
    self->total = len;
}

static void arena_block_free(struct ArenaBlock *const self) {
    free(self->ptr);
}

// If the block can accommodate `len` more bytes, allocates that in the block and returns 
// the address. Otherwise returns NULL
static void *arena_block_try_alloc(struct ArenaBlock *const self, usize const len) {
    if (self->used + len < self->total) {
        void *address = self->ptr + self->used;
        self->used += len;
        return address;
    } else {
        return NULL;
    }
}

static struct ArenaBlock *arena_add_block(struct Arena *const self) {
    self->block_count += 1;
    self->blocks = realloc(self->blocks, self->block_count);

    struct ArenaBlock *const new_block = &self->blocks[self->block_count - 1u];
    arena_block_init(new_block, self->block_len);
    return new_block;
}

void arena_init(struct Arena *const self, usize const block_len) {
    self->blocks = NULL;
    self->block_count = 0u;
    self->block_len = block_len;
}

void arena_free(struct Arena *const self) {
    arena_clear(self);
}

void *arena_alloc(struct Arena *const self, usize const len) {
    if (len > self->block_len) {
        log_error("cannot allocate %zu bytes in arena: len > block_len", len);
        exit(1);
    }

    for (usize i = 0u; i < self->block_count; i += 1) {
        struct ArenaBlock *const block = &self->blocks[i];
        void *address = arena_block_try_alloc(block, len);

        if (address != NULL) {
            return address;
        }
    }

    // Arena full
    struct ArenaBlock *const new_block = arena_add_block(self);
    return arena_block_try_alloc(new_block, len);
}

void *arena_copy(struct Arena *self, void const *item, usize item_size) {
    void *const ptr = arena_alloc(self, item_size);
    memcpy(ptr, item, item_size);
    return ptr;
}

void arena_clear(struct Arena *const self) {
    for (usize i = 0u; i < self->block_count; i += 1) {
        arena_block_free(&self->blocks[i]);
    }

    free(self->blocks);
    self->blocks = NULL;
}

void arenavec_init(
    struct ArenaVec *const self, 
    struct Arena *const backing_arena, 
    usize const element_size
) {
    self->backing_arena = backing_arena;
    self->element_size = element_size;
    self->data = NULL;
    self->len = 0u;
    self->cap = 0u;
}

void *arenavec_at(
    struct ArenaVec const *const self, 
    usize const index
) {
    if (index >= self->len) {
        log_error("arenavec_at: index out of range (index = %zu, len = %zu)", index, self->len);
        exit(1);
    }

    return self->data + index * self->element_size;
}

void arenavec_push(
    struct ArenaVec *const self, 
    void *const item
) {
    if (self->len + 1 > self->cap) {
        self->cap = (self->cap == 0u) 
            ? 16u // hardcoded initial capacity
            : self->cap * 2;

        self->data = arena_alloc(self->backing_arena, self->element_size * self->cap);
    }

    memcpy(self->data + self->len * self->element_size, item, self->element_size);
    self->len += 1;
}

