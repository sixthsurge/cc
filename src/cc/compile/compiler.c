#include "cc/compile/compiler.h"

#include <stdlib.h>

#include "cc/common.h"
#include "cc/compile/error.h"
#include "cc/compile/variable_table.h"
#include "cc/log.h"

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
    struct Type const type
) {
    // update stack offset
    usize const size_bytes = type_size_bytes(&type);
    self->stack_offset += size_bytes;
    self->stack_offset_temporary += size_bytes;
    self->stack_offset_max = max_usize(self->stack_offset_max, self->stack_offset);

    // update variable table
    *variable_desc_out = (struct VariableDescription) {
        .name = name,
        .type = type,
        .stack_offset = self->stack_offset,
    };

    return variable_table_update(self->variable_table, *variable_desc_out);
}
