#include "cc/vec.h"

#define VEC_ELEMENT_TYPE char 
#define VEC_TYPE CharVec 
#define VEC_SLICE_TYPE CharSlice 
#define VEC_FUNCTION_PREFIX charvec_
#include "cc/template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX


#define VEC_ELEMENT_TYPE void* 
#define VEC_TYPE PtrVec 
#define VEC_SLICE_TYPE PtrSlice
#define VEC_FUNCTION_PREFIX ptrvec_
#include "cc/template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX


#define VEC_ELEMENT_TYPE usize
#define VEC_TYPE UsizeVec 
#define VEC_SLICE_TYPE UsizeSlice
#define VEC_FUNCTION_PREFIX usizevec_
#include "cc/template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX

