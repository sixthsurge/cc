#pragma once

#include "cc/ast.h"
#include "cc/compile/assembly.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"

struct ExpressionValue {
    struct Operand operand; // Operand where the expression value is stored
    struct Type type; // Type of the expression value
};

struct CompileResult compile_expression(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstExpression const *ast
);
