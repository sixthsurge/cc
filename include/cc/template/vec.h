// define 
// VEC_TYPE            | Name for slice type e.g. CharVec
// VEC_ELEMENT_TYPE    | Type of slice elements e.g. char
// VEC_SLICE_TYPE      | Corresponding slice e.g. CharSlice
// VEC_FUNCTION_PREFIX | Prefix for functions on the slice, e.g. charvec_
// VEC_DEBUG_FN        | (Optional) function to print an element of the vec

#include "cc/common.h"

#define VEC_CONCAT1(a, b) a##b
#define VEC_CONCAT2(a, b) VEC_CONCAT1(a, b)
#define VEC_PREFIX(function) VEC_CONCAT2(VEC_FUNCTION_PREFIX, function)

struct Writer;

struct VEC_TYPE {
    usize len;
    usize capacity;
    VEC_ELEMENT_TYPE *data; 
};

void                     VEC_PREFIX(init)              (struct VEC_TYPE *self);
void                     VEC_PREFIX(init_with_capacity)(struct VEC_TYPE *self, usize capacity);
void                     VEC_PREFIX(clone)             (struct VEC_TYPE *self, struct VEC_TYPE const *other);
void                     VEC_PREFIX(init_from_slice)   (struct VEC_TYPE *self, struct VEC_SLICE_TYPE slice);
void                     VEC_PREFIX(free)              (struct VEC_TYPE *self);
VEC_ELEMENT_TYPE        *VEC_PREFIX(at)                (struct VEC_TYPE const *self, usize index);
VEC_ELEMENT_TYPE        *VEC_PREFIX(peek_back)         (struct VEC_TYPE const *self);
struct VEC_SLICE_TYPE    VEC_PREFIX(slice)             (struct VEC_TYPE const *self, usize begin, usize end);
struct VEC_SLICE_TYPE    VEC_PREFIX(slice_whole)       (struct VEC_TYPE const *self);
void                     VEC_PREFIX(resize)            (struct VEC_TYPE *self, usize new_capacity);
void                     VEC_PREFIX(reserve)           (struct VEC_TYPE *self, usize n);
void                     VEC_PREFIX(push)              (struct VEC_TYPE *self, VEC_ELEMENT_TYPE el);
void                     VEC_PREFIX(push_slice)        (struct VEC_TYPE *self, struct VEC_SLICE_TYPE slice);
void                     VEC_PREFIX(insert)            (struct VEC_TYPE *self, usize index, VEC_ELEMENT_TYPE el);
void                     VEC_PREFIX(insert_slice)      (struct VEC_TYPE *self, usize index, struct VEC_SLICE_TYPE slice);
void                     VEC_PREFIX(remove_at)         (struct VEC_TYPE *self, usize index);
void                     VEC_PREFIX(remove_range)      (struct VEC_TYPE *self, usize begin, usize end);
VEC_ELEMENT_TYPE         VEC_PREFIX(pop_back)          (struct VEC_TYPE *self);
void                     VEC_PREFIX(clear)             (struct VEC_TYPE *self);

#ifdef VEC_DEBUG_FN
void              VEC_PREFIX(debug)             (struct Writer *writer, struct VEC_TYPE const *self);
#endif

#undef VEC_CONCAT1
#undef VEC_CONCAT2
#undef VEC_PREFIX
