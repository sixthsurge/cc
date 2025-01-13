#pragma once

#include "common.h"

//--------------------------------------------------------------------------------
// Template instantiations
//--------------------------------------------------------------------------------

#define SLICE_ELEMENT_TYPE char 
#define SLICE_TYPE CharSlice 
#define SLICE_FUNCTION_PREFIX charslice_
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX


#define SLICE_ELEMENT_TYPE void* 
#define SLICE_TYPE PtrSlice 
#define SLICE_FUNCTION_PREFIX ptrslice_
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX


#define SLICE_ELEMENT_TYPE usize
#define SLICE_TYPE UsizeSlice 
#define SLICE_FUNCTION_PREFIX usizeslice_
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

//--------------------------------------------------------------------------------
// Specific helpers
//--------------------------------------------------------------------------------

char *charslice_as_cstr(struct CharSlice self);
struct CharSlice charslice_from_cstr(char const *cstr);

bool charslice_eq(struct CharSlice self, struct CharSlice other);
bool charslice_eq_cstr(struct CharSlice self, char const *cstr);
