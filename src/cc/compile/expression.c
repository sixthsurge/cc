#include "cc/compile/expression.h"
#include "cc/compile/assembly.h"
#include "cc/compile/error.h"
#include "cc/type.h"

struct CompileResult compile_expression(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstExpression const *ast
) {
    value_out->type.kind = TypeKindIntSigned32;
    value_out->operand = operand_immediate(DWord, 0);
    return compile_ok();
}
