#pragma once

#include "common.h"

struct Type;

enum TypeKind {
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
};

struct PointerType {
    struct Type const *pointee_type;
    bool is_const;
};

struct Type {
    enum TypeKind kind;

    union {
        struct PointerType pointer_type;
    };
};

