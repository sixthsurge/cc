#pragma once

#include "cc/compile/assembly.h"
#include "cc/type.h"

struct ArgumentLocationContext {
    usize int_argument_index;
    usize stack_displacement;
};

void argument_location_context_init(struct ArgumentLocationContext *self);

struct Operand locate_next_argument(
    struct ArgumentLocationContext *context,
    struct Type const *type
);
