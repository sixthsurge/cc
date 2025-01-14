#include "type.h"

#include <stdlib.h>

#include "log.h"

void type_debug(struct Writer *writer, struct Type const type) {
    switch (type.kind) {
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

usize type_size_bytes(struct Type const type) {
    switch (type.kind) {
        case TypeKindIntSigned32: return 4u;
        default: break;
    }

    log_error("unknown size for type kind %zu", (usize) type.kind);
    exit(1);
}
