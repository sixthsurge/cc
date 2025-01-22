#include "cc/compile/compiler.h"

#include <stdlib.h>

#include "cc/ast.h"
#include "cc/common.h"
#include "cc/compile/assembly.h"
#include "cc/compile/error.h"
#include "cc/compile/variable_table.h"
#include "cc/log.h"
#include "cc/type.h"

void compiler_push_scope(
    struct Compiler *const self, 
    struct VariableTable *const out_variable_table
) {
    variable_table_init(out_variable_table, self->variable_table);
    self->variable_table = out_variable_table;
}

void compiler_pop_scope(struct Compiler *const self) {
    if (self->variable_table->parent == NULL) {
        log_error("compiler_pop_scope called at global scope");
        exit(1);
    }

    struct VariableTable *const old = self->variable_table;
    self->variable_table = self->variable_table->parent;
    variable_table_free(old);
}

void compiler_init_function_context(
    struct Compiler *const self, 
    struct Type const *const return_type
) {
    self->function_return_type = return_type;
    self->stack_offset = 0u;
    self->stack_offset_max = 0u;
    self->stack_offset_temporary = 0u;
}

struct CompileResult compiler_declare_variable(
    struct Compiler *const self, 
    struct VariableDescription *variable_desc_out, 
    struct CharSlice const name, 
    struct Type const type,
    struct AstNodePosition const position
) {
    usize const type_size = type_size_bytes(&type);
    usize const type_alignment = type_alignment_bytes(&type);

    // update stack offset
    self->stack_offset = round_up_usize(self->stack_offset, type_alignment) + type_size;
    self->stack_offset_max = max_usize(self->stack_offset_max, self->stack_offset);
    self->stack_offset_temporary = self->stack_offset;

    // update variable table
    *variable_desc_out = (struct VariableDescription) {
        .name = name,
        .type = type,
        .stack_offset = self->stack_offset,
    };

    return variable_table_update(self->variable_table, *variable_desc_out, position);
}

struct Operand compiler_allocate_temporary_stack_space(
    struct Compiler *const self, 
    usize const size_bytes
) { 
    self->stack_offset_temporary += size_bytes;
    self->stack_offset_max = max_usize(self->stack_offset_temporary, self->stack_offset_max);
    return operand_stack(self->stack_offset_temporary);
}

void compiler_free_temporary_stack_space(
    struct Compiler *const self, 
    usize const size_bytes
) {
    if (self->stack_offset_temporary >= size_bytes) {
        self->stack_offset_temporary -= size_bytes;
    } else {
        log_error("compiler_free_temporary_stack_space: over-free");
        exit(1);
    }
}
