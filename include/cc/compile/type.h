#pragma once

#include "cc/ast.h"
#include "cc/common.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"
#include "cc/type.h"

struct CompileResult analyze_type(struct Type *out, struct AstType const *ast, struct Compiler *compiler);

