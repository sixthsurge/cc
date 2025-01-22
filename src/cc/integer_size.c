#include "cc/integer_size.h"

#include <stdlib.h>

#include "cc/log.h"

usize integer_size_bytes(enum IntegerSize const size) {
    switch (size) {
    case IntegerSize8:
        return 1u;
    case IntegerSize16:
        return 2u;
    case IntegerSize32:
        return 4u;
    case IntegerSize64:
        return 8u;
    default:
        log_error("integer_size_bytes: unknown integer size %zu", (usize) size);
        exit(1);
    }
}

char const *format_integer_size(enum IntegerSize const size) {
    switch (size) {
    case IntegerSize8:
        return "8";
    case IntegerSize16:
        return "16";
    case IntegerSize32:
        return "32";
    case IntegerSize64:
        return "64";
    default:
        return "<size unknown>";
    }
}

