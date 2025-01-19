#pragma once 

#include "cc/ast.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"

struct CompileResult compile_statement(struct Compiler *compiler, struct AstStatement const *ast);


