// define 
// VEC_TYPE            | Name for slice type e.g. CharVec
// VEC_ELEMENT_TYPE    | Type of slice elements e.g. char
// VEC_SLICE_TYPE      | Corresponding slice e.g. CharSlice
// VEC_FUNCTION_PREFIX | VEC_PREFIX for functions on the slice, e.g. charvec_

#define VEC_CONCAT1(a, b) a##b
#define VEC_CONCAT2(a, b) VEC_CONCAT1(a, b)
#define VEC_PREFIX(function) VEC_CONCAT2(VEC_FUNCTION_PREFIX, function)

#include "common.h"

typedef struct {
    usize len;
    usize capacity;
    VEC_ELEMENT_TYPE *data; 
} VEC_TYPE;

void              VEC_PREFIX(init)              (VEC_TYPE *self);
void              VEC_PREFIX(init_with_capacity)(VEC_TYPE *self, usize capacity);
void              VEC_PREFIX(clone)             (VEC_TYPE *self, VEC_TYPE const *other);
void              VEC_PREFIX(init_from_slice)   (VEC_TYPE *self, VEC_SLICE_TYPE slice);
void              VEC_PREFIX(free)              (VEC_TYPE *self);
VEC_ELEMENT_TYPE *VEC_PREFIX(at)                (VEC_TYPE const *self, usize index);
VEC_SLICE_TYPE    VEC_PREFIX(slice)             (VEC_TYPE const *self, usize begin, usize end);
VEC_SLICE_TYPE    VEC_PREFIX(slice_whole)       (VEC_TYPE const *self);
void              VEC_PREFIX(resize)            (VEC_TYPE *self, usize new_capacity);
void              VEC_PREFIX(reserve)           (VEC_TYPE *self, usize n);
void              VEC_PREFIX(push)              (VEC_TYPE *self, VEC_ELEMENT_TYPE el);
void              VEC_PREFIX(push_slice)        (VEC_TYPE *self, VEC_SLICE_TYPE slice);
void              VEC_PREFIX(insert)            (VEC_TYPE *self, usize index, VEC_ELEMENT_TYPE el);
void              VEC_PREFIX(insert_slice)      (VEC_TYPE *self, usize index, VEC_SLICE_TYPE slice);
void              VEC_PREFIX(remove_at)         (VEC_TYPE *self, usize index);
void              VEC_PREFIX(remove_range)      (VEC_TYPE *self, usize begin, usize end);
void              VEC_PREFIX(clear)             (VEC_TYPE *self);

#undef VEC_CONCAT1
#undef VEC_CONCAT2
#undef VEC_PREFIX
