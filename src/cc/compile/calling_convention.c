#include "cc/compile/calling_convention.h"

#include <stdlib.h>

#include "cc/compile/assembly.h"
#include "cc/log.h"
#include "cc/type.h"

// System V calling convention

void argument_location_context_init(struct ArgumentLocationContext *const self) {
    self->int_argument_index = 0u;
    self->stack_displacement = 16u;
}

struct ArgumentLocation locate_next_argument(
    struct ArgumentLocationContext *const context,
    struct Type const *const type
) {
    static enum IntRegister const first_int_registers[6u] = {
        RegisterDI,
        RegisterSI,
        RegisterD,
        RegisterC,
        Register8,
        Register9,
    };

    struct ArgumentLocation argument_location;

    if (type_is_integer_or_pointer(type) && context->int_argument_index < 6u) {
        argument_location.kind = ArgumentLocationIntRegister;
        argument_location.int_register = first_int_registers[context->int_argument_index];
        context->int_argument_index += 1u;
    } else {
        argument_location.kind = ArgumentLocationStack;
        argument_location.stack_displacement = context->stack_displacement;

        // TODO: make this based on the type lol
        context->stack_displacement += 8u;
    }

    return argument_location;
}

struct Operand argument_location_operand(
    enum OperandWidth const width, 
    struct ArgumentLocation const self
) {
    switch (self.kind) {
        case ArgumentLocationIntRegister: {
            return operand_register(width, self.int_register);
        }
        case ArgumentLocationStack: {
            return operand_memory(width, RegisterBP, self.stack_displacement);
        }
    }

    log_error("argument_location_operand: unknown argument location");
    exit(1);
}
