#include "compile.h"

#include <stdarg.h>

#include "assembly.h"
#include "ast.h"
#include "common.h"
#include "log.h"
#include "slice.h"
#include "stdlib.h"
#include "type.h"
#include "variable_table.h"
#include "vec.h"
#include "writer.h"

struct ExpressionValue {
    bool has_value;
    struct Operand operand;
    struct Type type;
};

struct CompileContext {
    struct VariableTable *variable_table;
    struct ExpressionValue last_expression_value;
    usize stack_offset;
    usize stack_offset_temporary;
    usize stack_offset_max;
};

ATTRIBUTE_PRINTF_LIKE(1, 2) 
static void compile_error(char const *const format_string, ...) {
    printf("%scompile error: %s", color_red, color_reset);
    va_list args;
    va_start(args, format_string);
    vprintf(format_string, args);
    va_end(args);
    exit(1);
}

static usize round_up_usize(usize value, usize round) {
    usize const floor = (value / round) * round;

    if (value == floor) {
        return value;
    } else {
        return floor + round;
    }
}

static struct Type get_type(struct AstType ast_type) {
    switch (ast_type.kind) {
        case AstTypeIntS32: {
            return (struct Type) {
                .kind = TypeKindIntSigned32,
            };
        }
    }

    log_error("unknown type kind %zu", (usize) ast_type.kind);
    exit(1);
} 

static void make_stack_space_for_variable(
    struct CompileContext *const context,
    usize const size_bytes
) {
    context->stack_offset += size_bytes;
    context->stack_offset_temporary += size_bytes;
    context->stack_offset_max = max_usize(context->stack_offset, context->stack_offset_max);
}

static void make_stack_space_for_temporary(
    struct CompileContext *const context,
    usize const size_bytes
) {
    context->stack_offset_temporary += size_bytes;
    context->stack_offset_max = max_usize(
        context->stack_offset_temporary, 
        context->stack_offset_max
    );
}

static struct VariableDescription try_declare_variable(
    struct CompileContext *const context,
    struct CharSlice const name,
    struct Type const type
) {
    if (variable_table_has(context->variable_table, name)) {
        char *const name_cstr = charslice_as_cstr(name);
        compile_error("redeclaration of variable %s", name_cstr);
        free(name_cstr);
    } 

    usize const size = type_size_bytes(type);
    make_stack_space_for_variable(context, size);

    struct VariableDescription const variable_desc = (struct VariableDescription) {
        .name = name,
        .type = type,
        .stack_offset = context->stack_offset,
    };

    variable_table_insert(context->variable_table, variable_desc);
    return variable_desc;
}

void compile_root(
    struct Writer *const assembly_writer, 
    struct AstRoot const *ast
) {
    struct VariableTable global_variable_table;
    variable_table_init(&global_variable_table, NULL);

    struct CompileContext context = {
        .variable_table = &global_variable_table,
        .last_expression_value = { .has_value = false },
        .stack_offset = 0u,
        .stack_offset_max = 0u,
        .stack_offset_temporary = 0u,
    };

    writer_write(assembly_writer, "global main\n");
    writer_write(assembly_writer, "section .text\n");

    for (usize item_index = 0u; item_index < ast->item_count; item_index += 1u) {
        compile_top_level_item(
            assembly_writer, 
            &ast->items[item_index], 
            &context
        );
    }

    variable_table_free(&global_variable_table);
}

void compile_top_level_item(
    struct Writer *const assembly_writer, 
    struct AstTopLevelItem const *const item,
    struct CompileContext *const context
) {
    switch (item->kind) {
        case AstTopLevelItemFunctionDefinition: {
            compile_function_definition(
                assembly_writer, 
                &item->function_definition, 
                context
            );
            break;
        }
        default: {
            struct CharVec msg;
            charvec_init(&msg);
            struct Writer msg_writer = charvec_writer(&msg);
            writer_write(&msg_writer, "don't know how to compile ");
            ast_debug_top_level_item(&msg_writer, item);
            char *const msg_cstr = charslice_as_cstr(charvec_slice_whole(&msg));
            log_error("%s", msg_cstr);
            free(msg_cstr);
            exit(1);
            break;
        }
    }
}

void compile_function_definition(
    struct Writer *const assembly_writer, 
    struct AstFunctionDefinition const *const function_definition,
    struct CompileContext *const context
) {
    // compile body
    struct CharVec body_instructions;
    charvec_init(&body_instructions);
    struct Writer body_writer = charvec_writer(&body_instructions);

    context->stack_offset = 0u;
    context->stack_offset_max = 0u;
    context->stack_offset_temporary = 0u;
    compile_block(
        &body_writer, 
        &function_definition->body, 
        context
    );

    // write label
    writer_write_charslice(assembly_writer, function_definition->signature.identifier.name);
    writer_write(assembly_writer, ":\n");

    // setup stack
    write_instruction(
        assembly_writer, 
        InstructionPush, 
        1u, 
        operand_register(QWord, RegisterBP)
    );
    write_instruction(
        assembly_writer, 
        InstructionMov, 
        2u, 
        operand_register(QWord, RegisterBP),
        operand_register(QWord, RegisterSP)
    );
    write_instruction(
        assembly_writer,
        InstructionSub,
        2u,
        operand_register(QWord, RegisterSP),
        operand_immediate(QWord, round_up_usize(context->stack_offset_max, 16u))
    );

    // body instructions
    writer_write_charslice(
        assembly_writer, 
        charvec_slice_whole(&body_instructions)
    );

    charvec_free(&body_instructions);
}

void compile_block(
    struct Writer *const assembly_writer, 
    struct AstBlock const *const block,
    struct CompileContext *const context
) {
    // variable table for this scope 
    struct VariableTable variable_table;
    variable_table_init(&variable_table, context->variable_table);
    context->variable_table = &variable_table;

    for (
        usize statement_index = 0u; 
        statement_index < block->statement_count; 
        statement_index += 1
    ) {
        struct AstStatement const *const statement = &block->statements[statement_index];
        compile_statement(assembly_writer, statement, context);
    }

    // reset variable table
    context->variable_table = variable_table.parent;
    variable_table_free(&variable_table);
}

void compile_statement(
    struct Writer *const assembly_writer, 
    struct AstStatement const *const statement,
    struct CompileContext *const context
) {
    context->stack_offset_temporary = context->stack_offset;

    switch (statement->kind) {
        case AstStatementExpression: {
            compile_expression(
                assembly_writer, 
                &statement->expression, 
                context
            );
            break;
        }
        case AstStatementVariableDeclaration: {
            struct VariableDescription const variable_desc = try_declare_variable(
                context, 
                statement->variable_declaration.identifier.name,
                get_type(statement->variable_declaration.type)
            );

            if (statement->variable_declaration.has_assigned_expression) {
                compile_expression(
                    assembly_writer, 
                    &statement->variable_declaration.assigned_expression, 
                    context
                );

                write_instruction(
                    assembly_writer, 
                    InstructionMov, 
                    2u, 
                    operand_stack(DWord, variable_desc.stack_offset),
                    context->last_expression_value.operand
                );
            }
            break;
        }
        case AstStatementReturn: {
            if (statement->return_statement.has_returned_expression) {
                compile_expression(
                    assembly_writer, 
                    &statement->return_statement.returned_expression, 
                    context
                );
                write_instruction(
                        assembly_writer,
                        InstructionMov,
                        2u,
                        operand_register(DWord, RegisterA),
                        context->last_expression_value.operand
                );
            }
            write_instruction(
                assembly_writer,
                InstructionLeave,
                0u
            );
            write_instruction(
                assembly_writer,
                InstructionRet,
                0u
            );
            break;
        }
        default: {
            struct CharVec msg;
            charvec_init(&msg);
            struct Writer msg_writer = charvec_writer(&msg);
            writer_write(&msg_writer, "don't know how to compile ");
            ast_debug_statement(&msg_writer, statement);
            char *const msg_cstr = charslice_as_cstr(charvec_slice_whole(&msg));
            log_error("%s", msg_cstr);
            free(msg_cstr);
            exit(1);
            break;
        }
    }
}

void compile_expression(
    struct Writer *const assembly_writer, 
    struct AstExpression const *const expression,
    struct CompileContext *const context
) {
    context->last_expression_value.has_value = false;

    switch (expression->kind) {
        case AstExpressionIdentifier: {
            compile_identifier(assembly_writer, &expression->identifier, context);
            break;
        }
        case AstExpressionConstant: {
            compile_constant(assembly_writer, &expression->constant, context);
            break;
        }
        case AstExpressionAssignment: {
            compile_assignment(assembly_writer, &expression->assignment, context);
            break;
        }
        case AstExpressionBinaryOp: {
            compile_binary_op(assembly_writer, &expression->binary_op, context);
            break;
        }
        default: {
            struct CharVec msg;
            charvec_init(&msg);
            struct Writer msg_writer = charvec_writer(&msg);
            writer_write(&msg_writer, "don't know how to compile ");
            ast_debug_expression(&msg_writer, expression);
            char *const msg_cstr = charslice_as_cstr(charvec_slice_whole(&msg));
            log_error("%s", msg_cstr);
            free(msg_cstr);
            exit(1);
            break;
        }
    }
}

void compile_identifier(
    struct Writer *const assembly_writer, 
    struct AstIdentifier const *const identifier,
    struct CompileContext *const context
) {
    struct VariableDescription variable_desc;
    if (!variable_table_lookup(context->variable_table, identifier->name, &variable_desc)) {
        char *const name_cstr = charslice_as_cstr(identifier->name);
        compile_error("undeclared identifier: %s", name_cstr);
        free(name_cstr);
    }

    context->last_expression_value.has_value = true;
    context->last_expression_value.operand = operand_stack(DWord, variable_desc.stack_offset);
    context->last_expression_value.type = variable_desc.type;
}

void compile_constant(
    struct Writer *const assembly_writer, 
    struct AstConstant const *const constant,
    struct CompileContext *const context
) {
    context->last_expression_value.has_value = true;
    context->last_expression_value.operand = operand_immediate(DWord, constant->value);
    context->last_expression_value.type = (struct Type) { .kind = TypeKindIntSigned32 };
}

void compile_assignment(
    struct Writer *const assembly_writer, 
    struct AstAssignment const *const assignment,
    struct CompileContext *const context
) {
    compile_identifier(
        assembly_writer, 
        &assignment->assignee.identifier, 
        context
    );
    struct ExpressionValue const assignee = context->last_expression_value;

    compile_expression(
        assembly_writer,
        assignment->assigned_expression,
        context
    );
    struct ExpressionValue const assigned = context->last_expression_value;

    // TODO: type checking and coercion
    // assumes assignee and assigned are DWord and assignee is in memory

    bool can_assign_directly = assigned.operand.kind == OperandRegister 
        || assigned.operand.kind == OperandImmediate;

    if (can_assign_directly) {
        write_instruction(
            assembly_writer, 
            InstructionMov, 
            2u, 
            assignee.operand,
            assigned.operand
        );
    } else {
        write_instruction(
            assembly_writer, 
            InstructionMov, 
            2u, 
            operand_register(DWord, RegisterA),
            assigned.operand
        );
        write_instruction(
            assembly_writer, 
            InstructionMov, 
            2u, 
            assignee.operand,
            operand_register(DWord, RegisterA)
        );
    }
}

void compile_binary_op(
    struct Writer *const assembly_writer, 
    struct AstBinaryOp const *const op,
    struct CompileContext *const context
) {
    compile_expression(assembly_writer, op->left, context);
    struct ExpressionValue left_value = context->last_expression_value;
    
    // if result is stored in a register, copy it to a temporary
    if (left_value.operand.kind == OperandRegister) {
        make_stack_space_for_temporary(context, 4u);
        struct Operand const new_operand = operand_stack(DWord, context->stack_offset_temporary);
        write_instruction(
            assembly_writer, 
            InstructionMov, 
            2u, 
            new_operand,
            left_value.operand
        );
        left_value.operand = new_operand;
    }

    compile_expression(assembly_writer, op->right, context);
    struct ExpressionValue right_value = context->last_expression_value;

    // if result is stored in a register, copy it to a temporary
    if (right_value.operand.kind == OperandRegister) {
        make_stack_space_for_temporary(context, 4u);
        struct Operand const new_operand = operand_stack(DWord, context->stack_offset_temporary);
        write_instruction(
            assembly_writer, 
            InstructionMov, 
            2u, 
            new_operand,
            right_value.operand
        );
        right_value.operand = new_operand;
    }

    switch (op->kind) {
        case AstBinaryOpAddition: {
            write_instruction(
                assembly_writer, 
                InstructionMov, 
                2u, 
                operand_register(DWord, RegisterA),
                left_value.operand
            );
            write_instruction(
                assembly_writer, 
                InstructionMov, 
                2u, 
                operand_register(DWord, RegisterB),
                right_value.operand
            );
            write_instruction(
                assembly_writer, 
                InstructionAdd, 
                2u, 
                operand_register(DWord, RegisterA),
                operand_register(DWord, RegisterB)
            );
            context->last_expression_value = (struct ExpressionValue) {
                .operand = operand_register(DWord, RegisterA),
                .type = (struct Type) { .kind = TypeKindIntSigned32 }
            };
            break;
        }
        default: {
            struct CharVec msg;
            charvec_init(&msg);
            struct Writer msg_writer = charvec_writer(&msg);
            writer_write(&msg_writer, "don't know how to compile ");
            ast_debug_binary_op(&msg_writer, op);
            char *const msg_cstr = charslice_as_cstr(charvec_slice_whole(&msg));
            log_error("%s", msg_cstr);
            free(msg_cstr);
            exit(1);
            break;
        }
    }
}
