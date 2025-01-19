#include "cc/function_signature.h"

#include <stdlib.h>
#include <string.h>

struct FunctionSignature function_signature_clone(struct FunctionSignature const *const other) {
    struct FunctionParameter *parameters = malloc(sizeof *parameters * other->parameter_count);
    memcpy(parameters, other->parameters, sizeof *parameters * other->parameter_count);

    return (struct FunctionSignature) {
        .return_type = other->return_type,
        .parameters = parameters,
        .parameter_count = other->parameter_count,
        .is_variadic = other->is_variadic,
    };
}

void function_signature_free(struct FunctionSignature *self) {
    free(self->parameters);
}

bool function_signatures_match(
    struct FunctionSignature const *self, 
    struct FunctionSignature const *other
) {
    if (self->parameter_count != other->parameter_count) {
        return false;
    }

    if (self->is_variadic != other->is_variadic) {
        return false;
    }

    if (!type_eq(&self->return_type, &other->return_type)) {
        return false;
    }

    for (
        usize parameter_index = 0u; 
        parameter_index < self->parameter_count;
        parameter_index += 1u
    ) {
        bool const types_match = type_eq(
            &self->parameters[parameter_index].type,
            &other->parameters[parameter_index].type
        );
        if (!types_match) { return false;
        }
    }

    return true;
}


