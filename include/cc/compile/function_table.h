#pragma once

#include "cc/function_signature.h"
#include "cc/map.h"
#include "cc/slice.h"

struct FunctionDescription {
    struct CharSlice name;
    struct FunctionSignature signature;
    bool has_definition;
};


// declare FunctionDescriptionSlice and FunctionDescriptionVec
#define SLICE_TYPE FunctionDescriptionSlice 
#define SLICE_ELEMENT_TYPE struct FunctionDescription 
#define SLICE_FUNCTION_PREFIX fdslice_
#include "cc/template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

#define VEC_TYPE FunctionDescriptionVec 
#define VEC_ELEMENT_TYPE struct FunctionDescription 
#define VEC_SLICE_TYPE FunctionDescriptionSlice
#define VEC_FUNCTION_PREFIX fdvec_
#include "cc/template/vec.h"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX

struct FunctionTable {
    struct Map__CharSlice_usize function_index;
    struct FunctionDescriptionVec function_descriptions;
};

void function_table_init(struct FunctionTable *self);
void function_table_free(struct FunctionTable *self);

bool function_table_has(struct FunctionTable *self, struct CharSlice name);

bool function_table_get(
    struct FunctionTable const *self, 
    struct CharSlice name, 
    struct FunctionDescription *out
); 

struct CompileResult function_table_declare(
    struct FunctionTable *self, 
    struct CharSlice name, 
    struct FunctionSignature const *signature
);
struct CompileResult function_table_define(
    struct FunctionTable *self, 
    struct CharSlice name, 
    struct FunctionSignature const *signature
);
