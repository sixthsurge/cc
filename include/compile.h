#pragma once 

#include "assembly.h"
#include "ast.h"
#include "common.h"
#include "type.h"

struct AstRoot;
struct Writer;

struct CompileContext;

void compile_root(
    struct Writer *assembly_writer, 
    struct AstRoot const *root
);

void compile_top_level_item(
    struct Writer *assembly_writer, 
    struct AstTopLevelItem const *top_level_item,
    struct CompileContext *context
);

void compile_function_definition(
    struct Writer *assembly_writer, 
    struct AstFunctionDefinition const *function_definition,
    struct CompileContext *context
);

void compile_block(
    struct Writer *assembly_writer, 
    struct AstBlock const *block,
    struct CompileContext *context
);

void compile_statement(
    struct Writer *assembly_writer, 
    struct AstStatement const *statement,
    struct CompileContext *context
);

void compile_expression(
    struct Writer *assembly_writer, 
    struct AstExpression const *expression,
    struct CompileContext *context
);

void compile_identifier(
    struct Writer *assembly_writer, 
    struct AstIdentifier const *identifier,
    struct CompileContext *context
);

void compile_constant(
    struct Writer *assembly_writer, 
    struct AstConstant const *constant,
    struct CompileContext *context
);

void compile_assignment(
    struct Writer *assembly_writer, 
    struct AstAssignment const *assignment,
    struct CompileContext *context
);

void compile_binary_op(
    struct Writer *assembly_writer, 
    struct AstBinaryOp const *op,
    struct CompileContext *context
);

