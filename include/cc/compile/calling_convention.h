#pragma once

#include "cc/compile/assembly.h"
#include "cc/type.h"

enum ArgumentLocationKind {
    ArgumentLocationIntRegister,
    ArgumentLocationStack
};

struct ArgumentLocation {
    enum ArgumentLocationKind kind;

    union {
        enum IntRegister int_register;
        i32 stack_displacement;
    };
};

struct ArgumentLocationContext {
    usize int_argument_index;
    usize stack_displacement;
};

void argument_location_context_init(struct ArgumentLocationContext *self);

struct ArgumentLocation locate_next_argument(
    struct ArgumentLocationContext *context,
    struct Type const *type
);

struct Operand argument_location_operand(enum OperandWidth width, struct ArgumentLocation self);
