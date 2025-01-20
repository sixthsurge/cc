#include "cc/compile/expression.h"

#include "cc/ast.h"
#include "cc/compile/assembly.h"
#include "cc/compile/error.h"
#include "cc/compile/variable_table.h"
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
            .undeclared_identifier = {
                .name = ast->name,
            },
        };
        return compile_error(error);
    }

    value_out->operand = operand_stack(DWord, variable_desc.stack_offset);
    value_out->type = variable_desc.type;

    return compile_ok();
}

static struct CompileResult compile_constant(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstConstant const *ast
) {
    value_out->operand = operand_immediate(DWord, ast->value);
    value_out->type.kind = TypeKindIntSigned32;
    return compile_ok();
}

struct CompileResult compile_expression(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstExpression const *ast
) {
    switch (ast->kind) {
        case AstExpressionIdentifier: {
            return compile_identifier(value_out, compiler, &ast->identifier);
        }
        case AstExpressionConstant: {
            return compile_constant(value_out, compiler, &ast->constant);
        }
        default: {
            return compile_error((struct CompileError) {
                .kind = CompileErrorNotImplemented,
                .position = ast->position,
            });
        }
    }
}

