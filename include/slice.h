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

//--------------------------------------------------------------------------------
// Specific helpers
//--------------------------------------------------------------------------------

char *charslice_as_cstr(CharSlice self);
CharSlice charslice_from_cstr(char const *cstr);

bool charslice_eq(CharSlice self, CharSlice other);
bool charslice_eq_cstr(CharSlice self, char const *cstr);
