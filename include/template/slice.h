// define 
// SLICE_TYPE            | Name for slice type e.g. CharSlice
// SLICE_ELEMENT_TYPE    | Type of slice elements e.g. char
// SLICE_FUNCTION_PREFIX | Prefix for functions on the slice, e.g. charslice_

#include "common.h"

typedef struct { 
    SLICE_ELEMENT_TYPE *ptr;
    usize len;
} SLICE_TYPE;

