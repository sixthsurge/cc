#include "cc/compile/function_definition.h"

#include "cc/compile/assembly.h"
#include "cc/compile/block.h"
#include "cc/compile/error.h"
#include "cc/compile/function_signature.h"
#include "cc/compile/function_table.h"
#include "cc/compile/variable_table.h"

struct CompileResult compile_function_definition(
    struct Compiler *const compiler, 
    struct AstFunctionDefinition const *const ast
) {
    // analyze function signature

    struct FunctionSignature signature;
    struct CompileResult signature_result 
        = analyze_function_signature(&signature, &ast->signature, compiler);

    if (!signature_result.ok) {
        return signature_result;
    }

    // register function definition

    struct CharSlice const name = ast->signature.identifier.name;
    struct CompileResult define_result 
        = function_table_define(compiler->function_table, name, &signature);

    if (!define_result.ok) {
        return define_result;
    }

    // prepare scope

    struct VariableTable variable_table;
    compiler_push_scope(compiler, &variable_table);

    compiler_init_function_context(compiler, &signature.return_type);

    // compile function body

    struct CharVec body_text;
    charvec_init(&body_text);

    compiler->writer_function_body = charvec_writer(&body_text);

    compile_block(compiler, &ast->body);

    // restore compiler state

    compiler_pop_scope(compiler);

    // emit code

    emit_label(&compiler->writer_text, name);
    emit_function_prologue(&compiler->writer_text, compiler->stack_offset_max);

    // cleanup

    function_signature_free(&signature);
    charvec_free(&body_text);

    return compile_ok();
}

