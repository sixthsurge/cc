#pragma once

#include "cc/common.h"
#include "cc/integer_size.h"

struct Type;
struct Writer;

enum TypeKind {
    TypeUnknown,
    TypeStruct,
    TypeEnum,
    TypeUnion,
    TypePointer,
    TypeVoid,
    TypeInteger,
    TypeFloat,
};

struct PointerType {
    struct Type const *pointee_type;
    bool is_const;
};

struct IntegerType {
    bool is_signed;
    enum IntegerSize size;
};

struct Type {
    enum TypeKind kind;

    union {
        struct PointerType pointer_type;
        struct IntegerType integer_type;
    } variant;
};

bool type_eq(struct Type const *self, struct Type const *other);
bool type_can_coerce(struct Type const *dst, struct Type const *src);
struct Type type_promote(struct Type first, struct Type second);
bool type_is_integer_or_pointer(struct Type const *self);
usize type_size_bytes(struct Type const *type);
usize type_alignment_bytes(struct Type const *type);
void type_debug(struct Writer *writer, struct Type const *type);


