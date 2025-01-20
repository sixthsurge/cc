#pragma once 

#include "cc/compile/expression.h"

struct CompileResult compile_call(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstCall const *ast
);

