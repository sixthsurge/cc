#pragma once 

#include "cc/compile/expression.h"

struct CompileResult compile_binary_op(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstBinaryOp const *ast
);
