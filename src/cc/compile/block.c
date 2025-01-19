#include "cc/compile/block.h"

#include "cc/compile/statement.h"

struct CompileResult compile_block(
    struct Compiler *compiler, 
    struct AstBlock const *ast
) {
    for (
        usize statement_index = 0u;
        statement_index < ast->statement_count;
        statement_index += 1u
    ) {
        struct AstStatement const *const statement = &ast->statements[statement_index];
        struct CompileResult const statement_result = compile_statement(compiler, statement);

        if (!statement_result.ok) {
            return statement_result;
        }
    }

    return compile_ok();
}

