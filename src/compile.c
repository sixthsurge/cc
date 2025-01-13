#include "compile.h"

#include "ast.h"
#include "map.h"
#include "slice.h"
#include "stdlib.h"
#include "writer.h"

#define VARIABLE_TABLE_INDEX_SIZE 91

struct VariableDescription {
    struct CharSlice name;
    struct Type type;
    usize stack_offset;
};

struct VariableTable {
    struct Map__CharSlice_usize variable_index;
    struct VariableDescription *variable_descriptions;
    usize variable_count;
    usize variable_descriptions_capacity;
};

static void variable_table_init(
    struct VariableTable *const self
) {
    map__charslice_usize__init(&self->variable_index, VARIABLE_TABLE_INDEX_SIZE);
    self->variable_descriptions = NULL;
    self->variable_count = 0u;
    self->variable_descriptions_capacity = 0u;
}

static void variable_table_free(
    struct VariableTable *const self 
) {
    map__charslice_usize__free(&self->variable_index);
    free(self->variable_descriptions);
}

static void variable_table_insert(
    struct VariableTable *const self,
    struct VariableDescription const description
) {
    // add to list of variable descriptions

    if (self->variable_descriptions == NULL) {
        self->variable_descriptions_capacity = 1u;
        self->variable_descriptions = malloc(sizeof (*self->variable_descriptions) * self->variable_descriptions_capacity);
    } else if (self->variable_count + 1u >= self->variable_descriptions_capacity) {
        self->variable_descriptions_capacity *= 2u;
        self->variable_descriptions = realloc(self->variable_descriptions, sizeof (*self->variable_descriptions) * self->variable_descriptions_capacity);
    }

    self->variable_descriptions[self->variable_count] = description;

    // add to index

    map__charslice_usize__set(
        &self->variable_index, 
        description.name,
        self->variable_count
    );

    // update variable count

    self->variable_count += 1;
}

static char const *word_kind_name(
    enum WordKind const word_kind
) {
    char const *const word_kind_names[4] = {
        "byte",
        "word",
        "dword",
        "qword"
    };

    return word_kind_names[word_kind];
}

static char const *register_name(
    enum Register const reg,
    enum WordKind const word_kind
) {
    char const *const register_names_byte[RegisterCount] = {
        "al",
        "bl",
        "cl",
        "dl",
        "sil",
        "dil",
        "spl",
        "bpl",
        "r8b",
        "r9b",
        "r10b",
        "r11b",
        "r12b",
        "r13b",
        "r14b",
        "r15b"
    };

    char const *const register_names_word[RegisterCount] = {
        "ax",
        "bx",
        "cx",
        "dx",
        "si",
        "di",
        "sp",
        "bp",
        "r8d",
        "r9d",
        "r10w",
        "r11w",
        "r12w",
        "r13w",
        "r14w",
        "r15w"
    };

    char const *const register_names_dword[RegisterCount] = {
        "eax",
        "ebx",
        "ecx",
        "edx",
        "esi",
        "edi",
        "esp",
        "ebp",
        "r8d",
        "r9d",
        "r10d",
        "r11d",
        "r12d",
        "r13d",
        "r14d",
        "r15d"
    };

    char const *const register_names_qword[RegisterCount] = {
        "rax",
        "rbx",
        "rcx",
        "rdx",
        "rsi",
        "rdi",
        "rsp",
        "rbp",
        "r8",
        "r9",
        "r10",
        "r11",
        "r12",
        "r13",
        "r14",
        "r15"
    };

    switch (word_kind) {
        case Byte: 
            return register_names_byte[reg];
        case Word: 
            return register_names_word[reg];
        case DWord: 
            return register_names_dword[reg];
        case QWord: 
            return register_names_qword[reg];
        default: 
            return "unknown word_kind";
    }
}

static struct Location location_register(
    enum Register const reg,
    enum WordKind const word_kind
) {
    return (struct Location) {
        .kind = LocationRegister,
        .reg = reg,
        .word_kind = word_kind,
    };
}

static struct Location location_stack(
    usize const stack_offset,
    enum WordKind const word_kind
) {
    return (struct Location) {
        .kind = LocationStack,
        .stack_offset = stack_offset,
        .word_kind = word_kind,
    };
}

static void write_location(
    struct Writer *const assembly_writer,
    struct Location const location
) {
    switch (location.kind) {
        case LocationStack:
            writer_writef(
                assembly_writer, 
                "%s [rbp-%zu]",
                word_kind_name(location.word_kind),
                location.stack_offset
            );
            break;

        case LocationRegister:
            writer_write(
                assembly_writer,
                register_name(location.reg, location.word_kind)
            );
            break;
    }
}

static void write_location_or_integer(
    struct Writer *const assembly_writer,
    struct LocationOrInteger const location_or_integer
) {
    if (location_or_integer.is_integer) {
        writer_writef(assembly_writer, "%zu", location_or_integer.integer);
    } else {
        write_location(assembly_writer, location_or_integer.location);
    }
}

static void write_mov(
    struct Writer *const assembly_writer,
    struct Location const dst,
    struct LocationOrInteger const src 
) {
    writer_write(assembly_writer, "\tmov ");
    write_location(assembly_writer, dst);
    writer_write(assembly_writer, ", ");
    write_location_or_integer(assembly_writer, src);
    writer_write(assembly_writer, "\n");
}

static struct VariableDescription *variable_table_get(
    struct VariableTable const *const self,
    struct CharSlice const name
) {
    usize const *const index = map__charslice_usize__get(&self->variable_index, name);

    if (index == NULL) { 
        return NULL;
    }

    return &self->variable_descriptions[*index];
}

static struct CompileResult compile_ok(void) {
    return (struct CompileResult) {
        .ok = true,
    };
}

struct CompileResult compile_root(
    struct Writer *assembly_writer, 
    struct AstRoot const *ast
) {
    struct CompileResult result = compile_ok();

    struct VariableTable variable_table;
    variable_table_init(&variable_table);

    writer_write(assembly_writer, "global main\n");

    writer_write(assembly_writer, "section .text\n");

    // main function
    writer_write(assembly_writer, "main:\n");
    writer_write(assembly_writer, "\tpush rbp\n");
    writer_write(assembly_writer, "\tmov rbp, rsp\n");

    // temp: reserve fixed stack amount
    writer_write(assembly_writer, "\tsub rsp, 64\n");

    usize stack_offset = 0u;

    for (
        usize statement_index = 0u; 
        statement_index < ast->block.statement_count; 
        statement_index += 1
    ) {
        struct AstStatement const statement = ast->block.statements[statement_index];
        struct LocationOrInteger expression_value;
        struct Type expression_type;

        switch (statement.kind) {
            case AstStatementExpression: {
                compile_expression(
                    assembly_writer, 
                    &statement.expression, 
                    &variable_table,
                    &expression_value,
                    &expression_type
                );
                break;
            }
            case AstStatementReturn: {
                compile_expression(
                    assembly_writer, 
                    &statement.return_statement.expression, 
                    &variable_table,
                    &expression_value,
                    &expression_type
                );
                write_mov(
                    assembly_writer,
                    location_register(RegisterA, DWord), 
                    expression_value
                );
                writer_write(
                    assembly_writer, 
                    "\tleave\n"
                );
                writer_write(
                    assembly_writer, 
                    "\tret\n"
                );

                goto End;
            }
            case AstStatementVariableDeclaration: {
                stack_offset += 4u;

                struct VariableDescription const variable_description = 
                    (struct VariableDescription) {
                        .name = statement.variable_declaration.identifier.name,
                        .stack_offset = stack_offset,
                    };

                variable_table_insert(
                    &variable_table, 
                    variable_description
                );

                if (statement.variable_declaration.has_assigned_expression) {
                    compile_expression(
                        assembly_writer, 
                        &statement.variable_declaration.assigned_expression, 
                        &variable_table,
                        &expression_value,
                        &expression_type
                    );
                    write_mov(
                        assembly_writer,
                        location_stack(variable_description.stack_offset, DWord), 
                        expression_value
                    );
                }

                break;
            }
        }
    }

End:
    variable_table_free(&variable_table);
    return compile_ok();
}

struct CompileResult compile_expression(
    struct Writer *assembly_writer, 
    struct AstExpression const *expression,
    struct VariableTable const *const variable_table,
    struct LocationOrInteger *const expression_value_out,
    struct Type *const expression_type_out
) {
    switch (expression->kind) {
        case AstExpressionConstant:
            return compile_constant(
                assembly_writer, 
                &expression->constant, 
                variable_table, 
                expression_value_out,
                expression_type_out
            );
        case AstExpressionIdentifier:
            return compile_identifier(
                assembly_writer, 
                &expression->identifier, 
                variable_table, 
                expression_value_out,
                expression_type_out
            );
        default:
            printf("don't know how to compile ");
            struct Writer stdout_writer = file_writer(stdout);
            ast_debug_expression(&stdout_writer, expression);
            break;
    }
}

struct CompileResult compile_identifier(
    struct Writer *assembly_writer, 
    struct AstIdentifier const *identifier,
    struct VariableTable const *const variable_table,
    struct LocationOrInteger *const expression_value_out,
    struct Type *const expression_type_out
) {
    struct VariableDescription const *const variable_description = variable_table_get(
        variable_table, 
        identifier->name
    );

    *expression_value_out = (struct LocationOrInteger) {
        .is_integer = false,
        .location = location_stack(variable_description->stack_offset, DWord),
    };

    return compile_ok();
}

struct CompileResult compile_constant(
    struct Writer *assembly_writer, 
    struct AstConstant const *constant,
    struct VariableTable const *const variable_table,
    struct LocationOrInteger *const expression_value_out,
    struct Type *const expression_type_out
) {
    *expression_value_out = (struct LocationOrInteger) {
        .is_integer = true,
        .integer = constant->value,
    };

    return compile_ok();
}
