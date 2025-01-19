#pragma once

#include "cc/slice.h"
#include "cc/type.h"

struct FunctionParameter {
    struct CharSlice name;
    struct Type type;
};

struct FunctionSignature {
    struct Type return_type;
    struct FunctionParameter *parameters;
    usize parameter_count;
    bool is_variadic; // whether the function takes variadic arguments (... parameter)
};

struct FunctionSignature function_signature_clone(struct FunctionSignature const *other);
void function_signature_free(struct FunctionSignature *self);

bool function_signatures_match(
    struct FunctionSignature const *left, 
    struct FunctionSignature const *right
);


