#include "cc/compile/root.h"

#include "cc/ast.h"
#include "cc/compile/error.h"
#include "cc/compile/function_definition.h"

struct CompileResult compile_root(struct Compiler *compiler, struct AstRoot const *const ast) {
    for (usize item_index = 0u; item_index < ast->item_count; item_index += 1u) {
        struct AstTopLevelItem const *const item = &ast->items[item_index];
        struct CompileResult result;

        switch (item->kind) {
            case AstTopLevelItemFunctionDefinition: {
                result = compile_function_definition(compiler, &item->variant.function_definition);
                break;
            }
        }

        if (!result.ok) {
            return result;
        }
    }

    return compile_ok();
}
