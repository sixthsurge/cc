#include "cc/compile/expression.h"

#include "cc/ast.h"
#include "cc/compile/assembly.h"
#include "cc/compile/binary_op.h"
#include "cc/compile/call.h"
#include "cc/compile/error.h"
#include "cc/compile/variable_table.h"
#include "cc/log.h"
#include "cc/type.h"

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
    value_out->operand = operand_immediate(ast->value);
    value_out->type.kind = TypeKindIntSigned32;
    return compile_ok();
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

    // TODO: type checking 

    // emit assignment 

    emit_assign_variable(
        &compiler->writer_function_body, 
        variable_desc.stack_offset, 
        variable_desc.type, 
        expression_value.operand, 
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
                case TypeKindIntSigned32: { 
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
                case TypeKindIntSigned32: { 
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
                case TypeKindIntSigned32: { 
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
                case TypeKindIntSigned32: { 
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

