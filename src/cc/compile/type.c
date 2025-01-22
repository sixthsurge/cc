#include "cc/compile/type.h"

#include <stdlib.h>

#include "cc/ast.h"
#include "cc/compile/error.h"
#include "cc/log.h"
#include "cc/type.h"

struct CompileResult analyze_type(
    struct Type *out, 
    struct AstType const *ast, 
    struct Compiler *compiler
) {
    switch (ast->kind) {
        case AstTypeInteger: {
            out->kind = TypeInteger;
            out->variant.integer_type = (struct IntegerType) {
                .size = ast->variant.integer_type.size,
                .is_signed = ast->variant.integer_type.is_signed
            };
            return compile_ok();
        }
    }
}
