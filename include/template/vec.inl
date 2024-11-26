// Define 
// VEC_TYPE            | Name for slice type e.g. CharVec
// VEC_ELEMENT_TYPE    | Type of slice elements e.g. char
// VEC_SLICE_TYPE      | Corresponding slice e.g. CharSlice
// VEC_FUNCTION_PREFIX | VEC_PREFIX for functions on the slice, e.g. charvec_

#include <stdlib.h>
#include <string.h>

#include "log.h"

#define VEC_CONCAT1(a, b) a##b
#define VEC_CONCAT2(a, b) VEC_CONCAT1(a, b)
#define VEC_PREFIX(function) VEC_CONCAT2(VEC_FUNCTION_PREFIX, function)

void VEC_PREFIX(init)(VEC_TYPE *const self) {
    self->len = 0u;
    self->capacity = 0u;

    self->data = (VEC_ELEMENT_TYPE *) NULL;
}

void VEC_PREFIX(init_with_capacity)(VEC_TYPE *const self, usize const capacity) {
    self->len = 0u;
    self->capacity = capacity;

    self->data = (VEC_ELEMENT_TYPE *) malloc(sizeof (VEC_ELEMENT_TYPE) * capacity);
}

void VEC_PREFIX(init_clone)(VEC_TYPE *const self, VEC_TYPE const *const other) {
    self->len = other->len;
    self->capacity = other->len;

    self->data = (VEC_ELEMENT_TYPE *) malloc(sizeof (VEC_ELEMENT_TYPE) * self->capacity);
    memcpy(self->data, other->data, sizeof (VEC_ELEMENT_TYPE) * self->len);
}

void VEC_PREFIX(init_from_slice)(VEC_TYPE *const self, VEC_SLICE_TYPE const slice) {
    self->len = slice.len;
    self->capacity = slice.len;

    self->data = (VEC_ELEMENT_TYPE *) malloc(sizeof (VEC_ELEMENT_TYPE) * self->capacity);
    memcpy(self->data, slice.ptr, sizeof (VEC_ELEMENT_TYPE) * slice.len);
}

void VEC_PREFIX(free)(VEC_TYPE *const self) {
    free(self->data);
}

VEC_ELEMENT_TYPE *VEC_PREFIX(at)(VEC_TYPE const *const self, usize const index) {
    if (index < self->len) {
        return &self->data[index];
    } else {
        log_error("vec_at: index out of range (index = %zu, length = %zu)", index, self->len);
        exit(1);
    }
}

VEC_SLICE_TYPE VEC_PREFIX(slice)(VEC_TYPE const *const self, usize const begin, usize const end) {
    if (begin > end) {
        log_error("vec_slice: begin > end (begin = %zu, end = %zu)", begin, end);
        exit(1);
    }
    if (end > self->len) {
        log_error("vec_slice: end index out of range (begin = %zu, end = %zu, length = %zu)", begin, end, self->len);
        exit(1);
    }

    return (VEC_SLICE_TYPE) {
        .ptr = self->data + begin,
        .len = end - begin,
    };
}

VEC_SLICE_TYPE VEC_PREFIX(slice_whole)(VEC_TYPE const *const self) {
    return (VEC_SLICE_TYPE) {
        .ptr = self->data,
        .len = self->len,
    };
}

void VEC_PREFIX(resize)(VEC_TYPE *const self, usize const new_capacity) {
    if (new_capacity < self->len) {
        log_error("vec_resize: shrinking beyond length (length = %zu, desired capacity = %zu)", self->len, new_capacity);
        exit(1);
    }

    if (self->data == NULL) {
        self->data = (VEC_ELEMENT_TYPE *) malloc(new_capacity * sizeof (VEC_ELEMENT_TYPE));
    } else {
        self->data = (VEC_ELEMENT_TYPE *) realloc(self->data, new_capacity * sizeof (VEC_ELEMENT_TYPE));
    }
}

void VEC_PREFIX(reserve)(VEC_TYPE *const self, usize const n) {
    usize min_capacity = self->len + n;

    if (min_capacity > self->capacity) {
        VEC_PREFIX(resize)(self, (min_capacity * 3u) / 2u);
    }
}

void VEC_PREFIX(push)(VEC_TYPE *const self, VEC_ELEMENT_TYPE const el) {
    VEC_PREFIX(reserve)(self, 1u);

    self->data[self->len] = el;
    self->len += 1u;
}

void VEC_PREFIX(push_slice)(VEC_TYPE *const self, VEC_SLICE_TYPE const slice) {
    VEC_PREFIX(reserve)(self, slice.len);

    memmove(self->data + self->len, slice.ptr, slice.len * sizeof (VEC_ELEMENT_TYPE));
    self->len += slice.len;
}

void VEC_PREFIX(insert)(VEC_TYPE *const self, usize const index, VEC_ELEMENT_TYPE const el) {
    if (index >= self->len) {
        log_error("vec_insert: index out of range (index = %zu, length = %zu)", index, self->len);
        exit(1);
    }

    VEC_PREFIX(reserve)(self, 1);
    memmove(
        self->data + index + 1, 
        self->data + index, 
        (self->len - index) * sizeof (VEC_ELEMENT_TYPE)
    );
    self->data[index] = el;
    self->len += 1;
}

void VEC_PREFIX(insert_slice)(VEC_TYPE *const self, usize const index, VEC_SLICE_TYPE const slice) {
    if (index >= self->len) {
        log_error("vec_insert_slice: index out of range (index = %zu, length = %zu)", index, self->len);
        exit(1);
    }

    VEC_PREFIX(reserve)(self, slice.len);
    memmove(
        self->data + index, 
        self->data + index, 
        (self->len - index) * sizeof (VEC_ELEMENT_TYPE)
    );
    memmove(self->data + index, slice.ptr, slice.len * sizeof (VEC_ELEMENT_TYPE));
    self->len += slice.len;
}

void VEC_PREFIX(remove_at)(VEC_TYPE *const self, usize const index) {
    if (index >= self->len) {
        log_error("vec_remove_at: index out of range (index = %zu, length = %zu)", index, self->len);
        exit(1);
    }

    memmove(self->data + index, self->data + index + 1, (self->len - index) * sizeof (VEC_ELEMENT_TYPE));
    self->len -= 1;
}

void VEC_PREFIX(remove_range)(VEC_TYPE *const self, usize begin, usize end) {
    if (begin > end) {
        log_error("vec_remove_range: begin > end (begin = %zu, end = %zu)", begin, end);
        exit(1);
    }
    if (end > self->len) {
        log_error("vec_remove_range: end index out of range (begin = %zu, end = %zu, length = %zu)", begin, end, self->len);
        exit(1);
    }

    usize const n = end - begin;

    memmove(self->data + begin, self->data + begin + n, (self->len - begin) * sizeof (VEC_ELEMENT_TYPE));
    self->len -= n;
}

#undef VEC_CONCAT1
#undef VEC_CONCAT2
#undef VEC_PREFIX