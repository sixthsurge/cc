#pragma once 

#include "cc/ast.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"

struct CompileResult compile_function_definition(struct Compiler *compiler, struct AstFunctionDefinition const *ast);

