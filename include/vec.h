#pragma once 

#include "slice.h"

#define VEC_TYPE CharVec 
#define VEC_ELEMENT_TYPE char 
#define VEC_SLICE_TYPE CharSlice
#define VEC_FUNCTION_PREFIX charvec_
#include "template/vec.h"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX


#define VEC_TYPE PtrVec 
#define VEC_ELEMENT_TYPE void* 
#define VEC_SLICE_TYPE PtrSlice
#define VEC_FUNCTION_PREFIX ptrvec_
#include "template/vec.h"
#undef VEC_TYPE
#undef VEC_ELEMENT_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX


#define VEC_TYPE UsizeVec 
#define VEC_ELEMENT_TYPE usize
#define VEC_SLICE_TYPE UsizeSlice
#define VEC_FUNCTION_PREFIX usizevec_
#include "template/vec.h"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX

