#pragma once 

#include "cc/ast.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"

struct CompileResult compile_block(struct Compiler *compiler, struct AstBlock const *ast);

