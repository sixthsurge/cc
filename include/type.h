#pragma once

#include "common.h"

struct Type;

typedef enum TypeKind {
    TypeKindStruct,
    TypeKindEnum,
    TypeKindUnion,
    TypeKindPointer,
    TypeKindVoid,
    TypeKindIntUnsigned8,
    TypeKindIntUnsigned16,
    TypeKindIntUnsigned32,
    TypeKindIntUnsigned64,
    TypeKindIntSigned8,
    TypeKindIntSigned16,
    TypeKindIntSigned32,
    TypeKindIntSigned64,
    TypeKindFloat32,
    TypeKindFloat64,
} TypeKind;

typedef struct PointerType {
    struct Type *pointee_type;
    bool is_const;
} PointerType;

typedef struct Type {
    TypeKind kind;

    union {
        PointerType pointer_type;
    };
} Type;


// Declare TypeVec and TypeSlice 

#define SLICE_TYPE TypeSlice 
#define SLICE_ELEMENT_TYPE Type
#define SLICE_FUNCTION_PREFIX typeslice_
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

#define VEC_TYPE TypeVec
#define VEC_ELEMENT_TYPE Type
#define VEC_SLICE_TYPE TypeSlice
#define VEC_FUNCTION_PREFIX typevec_
#include "template/vec.h"
#undef VEC_TYPE
#undef VEC_ELEMENT_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX

