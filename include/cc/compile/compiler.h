#pragma once

#include "cc/ast.h"
#include "cc/common.h"
#include "cc/compile/error.h"
#include "cc/compile/function_table.h"
#include "cc/compile/variable_table.h"
#include "cc/type.h"
#include "cc/writer.h"

struct Compiler {
    // Assembly writers
    struct Writer writer_text;
    struct Writer writer_data;
    struct Writer writer_function_body;
    // Symbol tables
    struct VariableTable *variable_table;
    struct FunctionTable *function_table;
    // Function context
    struct Type const *function_return_type;
    usize stack_offset;
    usize stack_offset_temporary;
    usize stack_offset_max;
};

void compiler_push_scope(struct Compiler *self, struct VariableTable *out_variable_table);
void compiler_pop_scope(struct Compiler *self);
void compiler_init_function_context(struct Compiler *self, struct Type const *return_type);
struct CompileResult compiler_declare_variable(
    struct Compiler *self, 
    struct VariableDescription *variable_desc_out, 
    struct CharSlice name, 
    struct Type type,
    struct AstNodePosition position
);
