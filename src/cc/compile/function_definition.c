#include "cc/compile/function_definition.h"

#include "cc/ast.h"
#include "cc/compile/assembly.h"
#include "cc/compile/block.h"
#include "cc/compile/calling_convention.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"
#include "cc/compile/function_signature.h"
#include "cc/compile/function_table.h"
#include "cc/compile/variable_table.h"
#include "cc/writer.h"

// declare function parameters in the variable table
// emit code to move all function parameters from their passed locations to the function's stack frame
static struct CompileResult compile_function_parameters(
    struct Compiler *const compiler,
    struct FunctionSignature const *const signature
) {
    struct ArgumentLocationContext argument_location_context;
    argument_location_context_init(&argument_location_context);

    for (
        usize parameter_index = 0u;
        parameter_index < signature->parameter_count;
        parameter_index += 1u
    ) {
        struct FunctionParameter const *const parameter = &signature->parameters[parameter_index];

        // declare variable

        struct VariableDescription variable_desc;
        struct CompileResult declare_result = compiler_declare_variable(
            compiler, 
            &variable_desc, 
            parameter->name, 
            parameter->type,
            parameter->ast_node_position
        );

        if (!declare_result.ok) return declare_result;

        // store passed argument in variable

        struct Operand const operand_src 
            = locate_next_argument(&argument_location_context, &variable_desc.type);

        emit_moves(
            &compiler->writer_function_body, 
            operand_stack(variable_desc.stack_offset), 
            operand_src, 
            DWord,
            RegisterA
        );
    }

    return compile_ok();
}

struct CompileResult compile_function_definition(
    struct Compiler *const compiler, 
    struct AstFunctionDefinition const *const ast
) {
    struct CompileResult result;

    // analyze function signature

    struct FunctionSignature signature;
    result = analyze_function_signature(&signature, &ast->signature, compiler);

    if (!result.ok) return result;

    // register function definition

    struct CharSlice const name = ast->signature.identifier.name;
    result = function_table_define(compiler->function_table, name, &signature, ast->position);

    if (!result.ok) return result;

    // prepare scope

    compiler_init_function_context(compiler, &signature.return_type);

    struct VariableTable variable_table;
    compiler_push_scope(compiler, &variable_table);

    // compile function body

    struct CharVec body_text;
    charvec_init(&body_text);

    compiler->writer_function_body = charvec_writer(&body_text);

    result = compile_function_parameters(compiler, &signature);
    if (!result.ok) return result;

    result = compile_block(compiler, &ast->body);
    if (!result.ok) return result;

    // add return if last statement is not return
    bool missing_return;
    if (ast->body.statement_count == 0u) {
        missing_return = true;
    } else {
        struct AstStatement const *const last_statement 
            = &ast->body.statements[ast->body.statement_count - 1u];
        missing_return = last_statement->kind != AstStatementReturn;
    }
    if (missing_return) {
        emit_function_exit(&compiler->writer_function_body);
    }

    // restore compiler state

    compiler_pop_scope(compiler);

    // emit code

    emit_label(&compiler->writer_text, name);
    emit_function_prologue(&compiler->writer_text, compiler->stack_offset_max);
    writer_write_charslice(&compiler->writer_text, charvec_slice_whole(&body_text));

    // cleanup

    function_signature_free(&signature);
    charvec_free(&body_text);

    return compile_ok();
}

