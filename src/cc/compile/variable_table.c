#include "cc/compile/variable_table.h"

#include "cc/ast.h"
#include "cc/compile/error.h"
#include "cc/hash.h"

#define MAP_TYPE            Map__CharSlice_VariableDescription
#define MAP_KEY_TYPE        struct CharSlice 
#define MAP_VALUE_TYPE      struct VariableDescription
#define MAP_FUNCTION_PREFIX map__charslice_variabledescription__
#define MAP_KEY_EQ_FN       charslice_eq
#define MAP_KEY_HASH_FN     charslice_hash_djb2
#include "cc/template/map.inl"
#undef MAP_TYPE            
#undef MAP_KEY_TYPE 
#undef MAP_VALUE_TYPE      
#undef MAP_FUNCTION_PREFIX 
#undef MAP_KEY_EQ_FN       
#undef MAP_KEY_HASH_FN     

#define VARIABLE_TABLE_INDEX_SIZE 91


void variable_table_init(
    struct VariableTable *const self, 
    struct VariableTable *const parent
) {
    map__charslice_variabledescription__init(&self->variable_index, VARIABLE_TABLE_INDEX_SIZE);
    self->parent = parent;
}

void variable_table_free(struct VariableTable *const self) {
    map__charslice_variabledescription__free(&self->variable_index);
}

struct CompileResult variable_table_update(
    struct VariableTable *const self, 
    struct VariableDescription const variable_desc,
    struct AstNodePosition const position
) {
    if (variable_table_has(self, variable_desc.name)) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorVariableRedeclaration,
            .position = position,
            .variable_redeclaration = {
                .name = variable_desc.name,
            }
        });
    }

    map__charslice_variabledescription__set(
        &self->variable_index,
        variable_desc.name,
        variable_desc
    );

    return compile_ok();
}

bool variable_table_has(struct VariableTable const *const self, struct CharSlice const name) {
    return map__charslice_variabledescription__contains_key(
        &self->variable_index,
        name
    );
}

bool variable_table_lookup(
    struct VariableTable const *const self, 
    struct CharSlice name, 
    struct VariableDescription *const out
) {
    struct VariableTable const *current = self;

    while (current != NULL) {
        struct VariableDescription const *const variable_description 
            = map__charslice_variabledescription__get(&current->variable_index, name);

        if (variable_description != NULL) {
            *out = *variable_description;
            return true;
        }

        current = current->parent;
    }

    return false;
}
