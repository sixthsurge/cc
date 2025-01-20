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
//  - left_value is not stored in a register
//  - right_value may be stored anywhere
//  - resulting value must be stored in register A

// emit "normal" binary operations; those consisting of a single instruction taking two operands 
// where the result is stored in the first operand
static void emit_normal_op(
    struct Writer *const assembly_writer,
    enum Instruction const instruction,
    enum OperandWidth const operand_width,
    struct Operand const left_operand,
    struct Operand const right_operand
) {
    emit_instruction(
        assembly_writer, 
        InstructionMov, 
        operand_width, 
        2u, 
        operand_register(RegisterB),
        right_operand
    );
    emit_instruction(
        assembly_writer, 
        InstructionMov, 
        operand_width, 
        2u, 
        operand_register(RegisterA),
        left_operand
    );
    emit_instruction(
        assembly_writer, 
        instruction, 
        operand_width, 
        2u, 
        operand_register(RegisterA),
        operand_register(RegisterB)
    );
}

static void emit_signed_div(
    struct Writer *const assembly_writer,
    enum OperandWidth const operand_width,
    struct Operand const left_operand,
    struct Operand const right_operand
) {
    emit_instruction(
        assembly_writer, 
        InstructionMov, 
        operand_width, 
        2u, 
        operand_register(RegisterB),
        right_operand
    );
    emit_instruction(
        assembly_writer, 
        InstructionMov, 
        operand_width, 
        2u, 
        operand_register(RegisterA),
        left_operand
    );
    emit_instruction(
        assembly_writer, 
        InstructionCdq, 
        QWord, 
        0u
    );
    emit_instruction(
        assembly_writer, 
        InstructionIDiv, 
        operand_width, 
        1u,
        operand_register(RegisterB)
    );
}

static void emit_add(
    struct Writer *const assembly_writer,
    struct Type const result_type,
    struct ExpressionValue const left_value,
    struct ExpressionValue const right_value
) {
    switch (result_type.kind) {
        case TypeKindIntSigned32: {
            emit_normal_op(
                assembly_writer,
                InstructionAdd,
                DWord,
                left_value.operand,
                right_value.operand
            );
            break;
        }
        default: {
            log_error("emit_add called with incorrect type kind: %zu", (usize) result_type.kind);
            exit(1);
        }
    }
}

static void emit_sub(
    struct Writer *const assembly_writer,
    struct Type const result_type,
    struct ExpressionValue const left_value,
    struct ExpressionValue const right_value
) {
    switch (result_type.kind) {
        case TypeKindIntSigned32: {
            emit_normal_op(
                assembly_writer,
                InstructionSub,
                DWord,
                left_value.operand,
                right_value.operand
            );
            break;
        }
        default: {
            log_error("emit_sub called with incorrect type kind: %zu", (usize) result_type.kind);
            exit(1);
        }
    }
}

static void emit_mul(
    struct Writer *const assembly_writer,
    struct Type const result_type,
    struct ExpressionValue const left_value,
    struct ExpressionValue const right_value
) {
    switch (result_type.kind) {
        case TypeKindIntSigned32: {
            emit_normal_op(
                assembly_writer,
                InstructionIMul,
                DWord,
                left_value.operand,
                right_value.operand
            );
            break;
        }
        default: {
            log_error("emit_mul called with incorrect type kind: %zu", (usize) result_type.kind);
            exit(1);
        }
    }
}

static void emit_div(
    struct Writer *const assembly_writer,
    struct Type const result_type,
    struct ExpressionValue const left_value,
    struct ExpressionValue const right_value
) {
    switch (result_type.kind) {
        case TypeKindIntSigned32: {
            emit_signed_div(
                assembly_writer,
                DWord,
                left_value.operand,
                right_value.operand
            );
            break;
        }
        default: {
            log_error("emit_div called with incorrect type kind: %zu", (usize) result_type.kind);
            exit(1);
        }
    }
}
struct CompileResult typecheck_binary_op(
    struct Type *const result_type,
    enum AstBinaryOpKind const op,
    struct Type left_type,
    struct Type right_type
) {
    // all types are ints and everything is allowed :)
    result_type->kind = TypeKindIntSigned32;
    return compile_ok();
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

        emit_moves(
            &compiler->writer_function_body, 
            operand_dst, 
            left_value.operand, 
            DWord, 
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
    result = typecheck_binary_op(&result_type, ast->kind, left_value.type, right_value.type);

    if (!result.ok) {
        return result;
    }

    // emit operation

    switch (ast->kind) {
        case AstBinaryOpAddition: {
            emit_add(
                &compiler->writer_function_body,
                result_type,
                left_value,
                right_value
            );
            break;
        }
        case AstBinaryOpSubtraction: {
            emit_sub(
                &compiler->writer_function_body,
                result_type,
                left_value,
                right_value
            );
            break;
        }
        case AstBinaryOpMultiplication: {
            emit_mul(
                &compiler->writer_function_body,
                result_type,
                left_value,
                right_value
            );
            break;
        }
        case AstBinaryOpDivision: {
            emit_div(
                &compiler->writer_function_body,
                result_type,
                left_value,
                right_value
            );
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

