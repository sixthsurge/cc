#include "vec.h"

#define VEC_ELEMENT_TYPE char 
#define VEC_TYPE CharVec 
#define VEC_SLICE_TYPE CharSlice 
#define VEC_FUNCTION_PREFIX charvec_
#include "template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
