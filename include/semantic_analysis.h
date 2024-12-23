#pragma once

#include "common.h"
#include "map.h"
#include "vec.h"

typedef struct Variable {
    Type type;
} Variable;

typedef struct Function {
    Type return_type;
    TypeVec argument_types;
} Function;

typedef enum SymbolKind {
    SymbolKindTypedef,
    SymbolKindStruct,
    SymbolKindUnion,
    SymbolKindEnum,
    SymbolKindVariable,
    SymbolKindFunction,
} SymbolKind;

typedef struct Symbol {
    SymbolKind kind;

    union {
        Type type;
        Variable variable;
        Function function;
    };
} Symbol;

typedef struct SymbolTable {
    struct SymbolTable const *parent;

    Map__CharSlice_usize items_index;
    Vec__Symbol items;

    Map__CharSlice_usize sub_tables_index;
    Vec__SymbolTable sub_tables;
} SymbolTable;
