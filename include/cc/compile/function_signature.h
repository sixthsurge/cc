#pragma once

#include "cc/ast.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"
#include "cc/function_signature.h"

struct CompileResult analyze_function_signature(
    struct FunctionSignature *out,
    struct AstFunctionSignature const *ast,
    struct Compiler *compiler
);
