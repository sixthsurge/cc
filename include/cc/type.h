#pragma once

#include "common.h"

struct Type;
struct Writer;

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

bool type_eq(struct Type const *self, struct Type const *other);
bool type_can_coerce(struct Type const *dst, struct Type const *src);
usize type_size_bytes(struct Type const *type);
void type_debug(struct Writer *writer, struct Type const *type);


