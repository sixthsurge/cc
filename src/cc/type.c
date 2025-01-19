#include "cc/compile/type.h"

#include <stdlib.h>

#include "cc/log.h"

void type_debug(struct Writer *writer, struct Type const *const type) {
    switch (type->kind) {
        case TypeKindIntSigned32: {
            writer_write(writer, "int");
            break;
        }
        default: {
            writer_write(writer, "??");
            break;
        }
    }
}

usize type_size_bytes(struct Type const *const type) {
    switch (type->kind) {
        case TypeKindIntSigned32: return 4u;
        default: break;
    }

    log_error("unknown size for type kind %zu", (usize) type->kind);
    exit(1);
}

bool type_eq(struct Type const *self, struct Type const *other) {
    // TEMP: all types are ints so all types are equal :)
    return true;
}

bool type_can_coerce(struct Type const *dst, struct Type const *src) {
    // TODO: type coercion
    return type_eq(src, dst);
}


