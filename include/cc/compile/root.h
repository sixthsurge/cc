#pragma once 

#include "cc/ast.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"

struct CompileResult compile_root(struct Compiler *compiler, struct AstRoot const *const ast);
