#include "cc/compile/binary_op.h"

#include <stdlib.h>

#include "cc/ast.h"
#include "cc/compile/assembly.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"
#include "cc/compile/expression.h"
#include "cc/log.h"
#include "cc/type.h"

// invariants for emit_ functions:
//  - left_value is stored in register A
//  - right_value is stored in register B
//  - resulting value must be stored in register A
static void emit_add(struct Writer *const assembly_writer, struct Type const type) {
    switch (type.kind) {
        case TypeInteger: {
            emit_instruction_dst_src(
                assembly_writer, 
                InstructionAdd, 
                integer_operand_width(type.variant.integer_type.size), 
                integer_operand_width(type.variant.integer_type.size), 
                operand_register(RegisterA),
                operand_register(RegisterB)
            );
            break;
        }
        default: {
            log_error("emit_add called with incorrect type kind: %zu", (usize) type.kind);
            exit(1);
        }
    }
}

static void emit_sub(struct Writer *const assembly_writer, struct Type const type) {
    switch (type.kind) {
        case TypeInteger: {
            emit_instruction_dst_src(
                assembly_writer, 
                InstructionSub, 
                integer_operand_width(type.variant.integer_type.size), 
                integer_operand_width(type.variant.integer_type.size), 
                operand_register(RegisterA),
                operand_register(RegisterB)
            );
            break;
        }
        default: {
            log_error("emit_sub called with incorrect type kind: %zu", (usize) type.kind);
            exit(1);
        }
    }
}

static void emit_mul(struct Writer *const assembly_writer, struct Type const type) {
    switch (type.kind) {
        case TypeInteger: {
            emit_instruction_dst_src(
                assembly_writer, 
                InstructionIMul, 
                integer_operand_width(type.variant.integer_type.size), 
                integer_operand_width(type.variant.integer_type.size), 
                operand_register(RegisterA),
                operand_register(RegisterB)
            );
            break;
        }
        default: {
            log_error("emit_mul called with incorrect type kind: %zu", (usize) type.kind);
            exit(1);
        }
    }
}

static void emit_div(struct Writer *const assembly_writer, struct Type const type) {
    switch (type.kind) {
        case TypeInteger: {
            emit_instruction(assembly_writer, InstructionCdq);
            emit_instruction_single_operand(
                assembly_writer, 
                InstructionIDiv, 
                integer_operand_width(type.variant.integer_type.size), 
                operand_register(RegisterB)
            );
            break;
        }
        default: {
            log_error("emit_div called with incorrect type kind: %zu", (usize) type.kind);
            exit(1);
        }
    }
}

bool typecheck_binary_op(
    struct Type *const result_type,
    enum AstBinaryOpKind const op,
    struct Type left_type,
    struct Type right_type
) {
    switch (op) {
        case AstBinaryOpAddition:
        case AstBinaryOpSubtraction:
        case AstBinaryOpMultiplication:
        case AstBinaryOpDivision: {
            *result_type = type_promote(left_type, right_type);
            return type_can_coerce(result_type, &left_type) && type_can_coerce(result_type, &right_type);
        }
        default: {
            return false;
        }
    }
}

struct CompileResult compile_binary_op(
    struct ExpressionValue *const value_out, 
    struct Compiler *const compiler, 
    struct AstBinaryOp const *const ast
) {
    usize temporary_stack_usage = 0u;
    struct CompileResult result;

    // compile left expression

    struct ExpressionValue left_value;
    result = compile_expression(&left_value, compiler, ast->left);

    if (!result.ok) {
        return result;
    }

    // if it is stored in a register, move left value to a temporary stack location
    if (left_value.operand.kind == OperandRegister) {
        usize const size_bytes = type_size_bytes(&left_value.type);
        struct Operand const operand_dst 
            = compiler_allocate_temporary_stack_space(compiler, size_bytes);

        emit_move(
            &compiler->writer_function_body, 
            operand_dst, 
            left_value.operand, 
            integer_operand_width(left_value.type.variant.integer_type.size),
            integer_operand_width(left_value.type.variant.integer_type.size),
            RegisterA
        );

        left_value.operand = operand_dst;
        temporary_stack_usage += size_bytes;
    }

    // compile right expression 

    struct ExpressionValue right_value;
    result = compile_expression(&right_value, compiler, ast->right);

    if (!result.ok) {
        return result;
    }

    // type checking

    struct Type result_type;
    bool typecheck_ok = typecheck_binary_op(&result_type, ast->kind, left_value.type, right_value.type);

    if (!typecheck_ok) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorIncompatibleTypesWithBinaryOp,
            .position = ast->position,
            .variant.incompatible_types_with_binary_op = {
                .op = ast->kind,
                .first = left_value.type,
                .second = left_value.type,
            },
        });
    }

    // move values to registers and perform type conversion

    emit_assignment(
        &compiler->writer_function_body, 
        operand_register(RegisterB), 
        right_value.operand, 
        result_type, 
        right_value.type
    );
    emit_assignment(
        &compiler->writer_function_body, 
        operand_register(RegisterA), 
        left_value.operand, 
        result_type, 
        left_value.type
    );

    // emit operation

    switch (ast->kind) {
        case AstBinaryOpAddition: {
            emit_add(&compiler->writer_function_body, result_type);
            break;
        }
        case AstBinaryOpSubtraction: {
            emit_sub(&compiler->writer_function_body, result_type);
            break;
        }
        case AstBinaryOpMultiplication: {
            emit_mul(&compiler->writer_function_body, result_type);
            break;
        }
        case AstBinaryOpDivision: {
            emit_div(&compiler->writer_function_body, result_type);
            break;
        }
        default: {
            return compile_error((struct CompileError) {
                .kind = CompileErrorNotImplemented,
                .position = ast->position,
            });
        }
    }

    compiler_free_temporary_stack_space(compiler, temporary_stack_usage);

    value_out->operand = operand_register(RegisterA);
    value_out->type = result_type;

    return compile_ok();
}

