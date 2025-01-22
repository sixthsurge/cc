#include "cc/compile/expression.h"

#include "cc/ast.h"
#include "cc/compile/assembly.h"
#include "cc/compile/binary_op.h"
#include "cc/compile/call.h"
#include "cc/compile/error.h"
#include "cc/compile/variable_table.h"
#include "cc/log.h"
#include "cc/type.h"

static enum IntegerSize get_integer_size_for_constant(u64 value, bool is_signed) {
    u64 const max_32 = is_signed ? INT32_MAX : UINT32_MAX;

    if (value < max_32) {
        return IntegerSize32;
    } else {
        return IntegerSize64;
    }
}

static struct CompileResult compile_identifier(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstIdentifier const *ast
) {
    struct VariableDescription variable_desc;
    bool exists = variable_table_lookup(
        compiler->variable_table, 
        ast->name, 
        &variable_desc
    );

    if (!exists) {
        struct CompileError const error = {
            .kind = CompileErrorUndeclaredIdentifier,
            .position = ast->position,
            .variant.undeclared_identifier = {
                .name = ast->name,
            },
        };
        return compile_error(error);
    }

    value_out->operand = operand_stack(variable_desc.stack_offset);
    value_out->type = variable_desc.type;

    return compile_ok();
}

static struct CompileResult compile_constant(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstConstant const *ast
) {
    switch (ast->kind) {
        case AstConstantInteger: {
            bool const is_signed = ast->variant.integer.is_signed;
            enum IntegerSize const size = ast->variant.integer.is_long
                ? IntegerSize64
                : get_integer_size_for_constant(ast->variant.integer.value, is_signed);

            value_out->operand = operand_immediate(ast->variant.integer.value);
            value_out->type = (struct Type) {
                .kind = TypeInteger,
                .variant.integer_type = {
                    .size = size,
                    .is_signed = is_signed,
                },
            };

            return compile_ok();
        }
        default: {
            return compile_error((struct CompileError) {
                .kind = CompileErrorNotImplemented,
                .position = ast->position,
            });
        }
    }
}

static struct CompileResult compile_assignment(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstAssignment const *ast
) {
    struct CompileResult result;

    // analyze identifier

    struct VariableDescription variable_desc;
    bool exists = variable_table_lookup(
        compiler->variable_table, 
        ast->assignee.identifier.name, 
        &variable_desc
    );
    if (!exists) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorUndeclaredIdentifier,
            .position = ast->assignee.position,
            .variant.undeclared_identifier = {
                .name = ast->assignee.identifier.name,
            },
        });
    }

    // compile expression 

    struct ExpressionValue expression_value;
    compile_expression(&expression_value, compiler, ast->assigned_expression);

    // type checking 

    if (!type_can_coerce(&variable_desc.type, &expression_value.type)) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorIncompatibleTypes,
            .position = ast->position,
            .variant.incompatible_types = {
                .first = variable_desc.type,
                .second = expression_value.type,
            },
        });
    }

    // emit assignment 

    emit_assignment(
        &compiler->writer_function_body, 
        operand_stack(variable_desc.stack_offset), 
        expression_value.operand, 
        variable_desc.type, 
        expression_value.type
    );

    value_out->operand = operand_stack(variable_desc.stack_offset);
    value_out->type = variable_desc.type;

    return compile_ok();
}

static bool analyze_binary_op(
    enum Instruction *const instruction_out,
    struct Type *const result_type_out, 
    enum AstBinaryOpKind const op,
    struct Type const left_type, 
    struct Type const right_type
) {
    switch (op) {
        case AstBinaryOpAddition: {
            switch (left_type.kind) {
                case TypeInteger: { 
                    *instruction_out = InstructionAdd;
                    return true;
                }
                default: { 
                    return false;
                }
            }
        }
        case AstBinaryOpSubtraction: {
            switch (left_type.kind) {
                case TypeInteger: { 
                    *instruction_out = InstructionSub;
                    return true;
                }
                default: { 
                    return false;
                }
            }
        }
        case AstBinaryOpMultiplication: {
            switch (left_type.kind) {
                case TypeInteger: { 
                    *instruction_out = InstructionIMul;
                    return true;
                }
                default: { 
                    return false;
                }
            }
        }
        case AstBinaryOpDivision: {
            switch (left_type.kind) {
                case TypeInteger: { 
                    *instruction_out = InstructionIDiv;
                    return true;
                }
                default: { 
                    return false;
                }
            }
        }
        default: {
            log_error("analyze_binary_op: unknown op kind %zu", (usize) op);
            return false;
        }
    }
}

struct CompileResult compile_expression(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstExpression const *ast
) {
    switch (ast->kind) {
        case AstExpressionIdentifier: {
            return compile_identifier(value_out, compiler, &ast->variant.identifier);
        }
        case AstExpressionConstant: {
            return compile_constant(value_out, compiler, &ast->variant.constant);
        }
        case AstExpressionCall: {
            return compile_call(value_out, compiler, &ast->variant.call);
        }
        case AstExpressionAssignment: {
            return compile_assignment(value_out, compiler, &ast->variant.assignment);
        }
        case AstExpressionBinaryOp: {
            return compile_binary_op(value_out, compiler, &ast->variant.binary_op);
        }
        default: {
            return compile_error((struct CompileError) {
                .kind = CompileErrorNotImplemented,
                .position = ast->position,
            });
        }
    }
}

