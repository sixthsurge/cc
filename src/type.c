#include "type.h"

// Define TypeVec and TypeSlice 

#define SLICE_TYPE TypeSlice 
#define SLICE_ELEMENT_TYPE Type
#define SLICE_FUNCTION_PREFIX typeslice_
#include "template/slice.inl"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

#define VEC_TYPE TypeVec
#define VEC_ELEMENT_TYPE Type
#define VEC_SLICE_TYPE TypeSlice
#define VEC_FUNCTION_PREFIX typevec_
#include "template/vec.inl"
#undef VEC_TYPE
#undef VEC_ELEMENT_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX

