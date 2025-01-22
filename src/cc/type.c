#include "cc/compile/type.h"

#include <stdlib.h>

#include "cc/integer_size.h"
#include "cc/log.h"
#include "cc/type.h"

void type_debug(struct Writer *writer, struct Type const *const type) {
    switch (type->kind) {
        case TypeInteger: {
            struct IntegerType const *const integer_type = &type->variant.integer_type;
            
            if (!integer_type->is_signed) {
                writer_write(writer, "u");
            }

            writer_writef(
                writer, 
                "int%s", 
                format_integer_size(integer_type->size)
            );
            break;
        }
        default: {
            writer_write(writer, "??");
            break;
        }
    }
}

struct Type type_promote(
    struct Type first, 
    struct Type second
) {
    if (first.kind == TypeInteger && second.kind == TypeInteger) {
        // type which can represent the highest positive value is the choice
        usize const first_value = 2u * (usize) first.variant.integer_type.size 
            + (usize) !first.variant.integer_type.is_signed;

        usize const second_value = 2u * (usize) second.variant.integer_type.size 
            + (usize) !second.variant.integer_type.is_signed;

        struct Type result = (first_value >= second_value)
            ? first
            : second;

        // all ints smaller than `int` are promoted to `int`
        if (result.variant.integer_type.size < IntegerSize32) {
            result.variant.integer_type.size = IntegerSize32;
        }

        return result;
    } else {
        return first;
    }
}

usize type_size_bytes(struct Type const *const type) {
    switch (type->kind) {
        case TypeInteger: {
            return integer_size_bytes(type->variant.integer_type.size);
        }
    }

    log_error("type_size_bytes: not implemented for type kind %zu", (usize) type->kind);
    exit(1);
}

usize type_alignment_bytes(struct Type const *const type) {
    switch (type->kind) {
        case TypeInteger: {
            return integer_size_bytes(type->variant.integer_type.size);
        }
    }

    log_error("type_size_bytes: not implemented for type kind %zu", (usize) type->kind);
    exit(1);
}

bool type_is_integer_or_pointer(struct Type const *self) {
    return true;
}

bool type_eq(struct Type const *self, struct Type const *other) {
    if (self->kind != other->kind) {
        return false;
    }

    switch (self->kind) {
        case TypeInteger: {
            return self->variant.integer_type.size == other->variant.integer_type.size
                && self->variant.integer_type.is_signed == other->variant.integer_type.is_signed;
        }
        default: {
            log_error("type_eq: unsupported type kind %zu", (usize) self->kind);
            exit(1);
        }
    }
}

bool type_can_coerce(struct Type const *dst, struct Type const *src) {
    if (type_eq(src, dst)) {
        return true;
    }
    if (src->kind == TypeInteger && dst->kind == TypeInteger) {
        return true;
    }
    if (src->kind == TypeInteger && dst->kind == TypeFloat) {
        return true;
    }
    if (src->kind == TypePointer && dst->kind == TypePointer) {
        return true;
    }

    return false;
}


