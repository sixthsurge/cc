#pragma once 

#include "hash.h"
#include "slice.h"
#include "type.h"

struct VariableDescription {
    struct CharSlice name;
    struct Type type;
    usize stack_offset;
};

#define MAP_TYPE            Map__CharSlice_VariableDescription
#define MAP_KEY_TYPE        struct CharSlice 
#define MAP_VALUE_TYPE      struct VariableDescription
#define MAP_FUNCTION_PREFIX map__charslice_variabledescription__
#define MAP_KEY_EQ_FN       charslice_eq
#define MAP_KEY_HASH_FN     charslice_hash_djb2
#include "template/map.h"
#undef MAP_TYPE            
#undef MAP_KEY_TYPE 
#undef MAP_VALUE_TYPE      
#undef MAP_FUNCTION_PREFIX 
#undef MAP_KEY_EQ_FN       
#undef MAP_KEY_HASH_FN     


struct VariableTable {
    struct Map__CharSlice_VariableDescription variable_index; 
    struct VariableTable *parent;
};

void variable_table_init(struct VariableTable *self, struct VariableTable *parent);
void variable_table_free(struct VariableTable *self);
void variable_table_insert(struct VariableTable *self, struct VariableDescription variable_desc);
bool variable_table_has(struct VariableTable const *self, struct CharSlice name);
bool variable_table_lookup(struct VariableTable const *self, struct CharSlice name, struct VariableDescription *out); 
