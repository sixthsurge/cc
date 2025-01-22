#pragma once

#include "common.h"

enum IntegerSize {
    IntegerSizeUnknown,
    IntegerSize8,
    IntegerSize16,
    IntegerSize32,
    IntegerSize64,
};

usize integer_size_bytes(enum IntegerSize const size);

char const *format_integer_size(enum IntegerSize const size);
