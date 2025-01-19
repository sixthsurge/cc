#pragma once 

#include "ast.h"
#include "writer.h"
#include "compile/error.h"

struct CompileResult compile(struct Writer *assembly_writer, struct AstRoot *const ast);
