#include "cc/compile/type.h"

#include <stdlib.h>

#include "cc/compile/error.h"
#include "cc/log.h"

struct CompileResult analyze_type(struct Type *out, struct AstType const *ast, struct Compiler *compiler) {
    // temp: only type is int
    *out = (struct Type) { .kind = TypeKindIntSigned32 };
    return compile_ok();
}
