#pragma once 

#include "ast.h"
#include "common.h"
#include "type.h"

struct AstRoot;
struct Writer;
struct VariableTable;

struct CompileResult {
    bool ok;
};

enum WordKind {
    Byte,
    Word,
    DWord,
    QWord
};

enum Register {
    RegisterA,
    RegisterB,
    RegisterC,
    RegisterD,
    RegisterSI,
    RegisterDI,
    RegisterSP,
    RegisterBP,
    Register8,
    Register9,
    Register10,
    Register11,
    Register12,
    Register13,
    Register14,
    Register15,
    RegisterCount
};

enum LocationKind {
    LocationStack,
    LocationRegister
};

struct Location {
    enum LocationKind kind;
    enum WordKind word_kind;

    union {
        usize stack_offset;
        enum Register reg;
    };
};

struct LocationOrInteger {
    bool is_integer;

    union {
        struct Location location;
        usize integer;
    };
};

struct CompileResult compile_root(
    struct Writer *assembly_writer, 
    struct AstRoot const *root
);

struct CompileResult compile_expression(
    struct Writer *assembly_writer, 
    struct AstExpression const *expression,
    struct VariableTable const *const variable_table,
    struct LocationOrInteger *const expression_value_out,
    struct Type *const expression_type_out
);

struct CompileResult compile_constant(
    struct Writer *assembly_writer, 
    struct AstConstant const *constant,
    struct VariableTable const *const variable_table,
    struct LocationOrInteger *const expression_value_out,
    struct Type *const expression_type_out
);

struct CompileResult compile_identifier(
    struct Writer *assembly_writer, 
    struct AstIdentifier const *identifier,
    struct VariableTable const *const variable_table,
    struct LocationOrInteger *const expression_value_out,
    struct Type *const expression_type_out
);
