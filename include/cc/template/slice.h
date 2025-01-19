// define 
// SLICE_TYPE            | Name for slice type e.g. CharSlice
// SLICE_ELEMENT_TYPE    | Type of slice elements e.g. char
// SLICE_FUNCTION_PREFIX | Prefix for functions on the slice, e.g. charslice_
// SLICE_DEBUG_FN        | (Optional) function to print an element of the slice

#include "cc/common.h"

#define SLICE_CONCAT1(a, b) a##b
#define SLICE_CONCAT2(a, b) SLICE_CONCAT1(a, b)
#define SLICE_PREFIX(function) SLICE_CONCAT2(SLICE_FUNCTION_PREFIX, function)

struct SLICE_TYPE { 
    SLICE_ELEMENT_TYPE *ptr;
    usize len;
};

#ifdef SLICE_DEBUG_FN
struct Writer;
void SLICE_PREFIX(debug)(struct Writer *writer, struct SLICE_TYPE const *self);
#endif

#undef SLICE_CONCAT1
#undef SLICE_CONCAT2
#undef SLICE_PREFIX
