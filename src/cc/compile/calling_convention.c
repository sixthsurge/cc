#include "cc/compile/calling_convention.h"

#include <stdlib.h>

#include "cc/compile/assembly.h"
#include "cc/log.h"
#include "cc/type.h"

// System V calling convention

void argument_location_context_init(struct ArgumentLocationContext *const self) {
    self->int_argument_index = 0u;
    self->stack_displacement = 0u;
}

struct Operand locate_next_argument(
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

    if (type_is_integer_or_pointer(type) && context->int_argument_index < 6u) {
        struct Operand const operand 
            = operand_register(first_int_registers[context->int_argument_index]);
        context->int_argument_index += 1u;
        return operand;
    } else {
        struct Operand const operand 
            = operand_memory(RegisterBP, context->stack_displacement + 16u);

        // TODO: make this based on the type lol
        context->stack_displacement += 8u;
        return operand;
    }
}
