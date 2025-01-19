#include "cc/compile/statement.h"

#include "cc/ast.h"
#include "cc/compile/error.h"
#include "cc/compile/expression.h"
#include "cc/compile/type.h"
#include "cc/compile/variable_table.h"

static struct CompileResult compile_variable_declaration(
    struct Compiler *const compiler, 
    struct AstVariableDeclaration const *const ast
) {
    // analyze type
    
    struct Type type;
    struct CompileResult type_result = analyze_type(&type, &ast->type, compiler);

    if (!type_result.ok) {
        return type_result;
    }

    // declare variable
    
    struct VariableDescription variable_desc;
    struct CompileResult declare_result = compiler_declare_variable(
        compiler, 
        &variable_desc,
        ast->identifier.name,
        type
    );

    if (!declare_result.ok) {
        return declare_result;
    }

    // compile assigned expression

    if (ast->has_assigned_expression) {
        struct ExpressionValue expression_value;
        struct CompileResult expression_result 
            = compile_expression(&expression_value, compiler, &ast->assigned_expression);

        if (!expression_result.ok) {
            return expression_result;
        }
        if (!type_can_coerce(&type, &expression_value.type)) {
            return compile_error((struct CompileError) {
                .kind = CompileErrorIncompatibleTypes,
                .incompatible_types = {
                    .first = type,
                    .second = expression_value.type,
                },
            });
        }

        emit_assign_variable(
            &compiler->writer_function_body, 
            variable_desc.stack_offset,
            variable_desc.type,
            expression_value.operand,
            expression_value.type
        );
    }

    return compile_ok();
}

static struct CompileResult compile_return_statement(
    struct Compiler *const compiler, 
    struct AstReturn const *const ast,
    struct Type const *const return_type
) {
    return compile_ok();
}

struct CompileResult compile_statement(
    struct Compiler *const compiler, 
    struct AstStatement const *const ast
) {
    switch (ast->kind) {
        case AstStatementExpression: {
            struct ExpressionValue expression_value_unused;
            return compile_expression(&expression_value_unused, compiler, &ast->expression);
        }
        case AstStatementVariableDeclaration: {
            return compile_variable_declaration(compiler, &ast->variable_declaration);
        }
        case AstStatementReturn: {
            return compile_return_statement(compiler, &ast->return_statement);
        }
        default: {
            return compile_error((struct CompileError) {
                .kind = CompileErrorNotImplemented,
            });
        }
    }
}

