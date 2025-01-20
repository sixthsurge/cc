#include "cc/compile/function_signature.h"

#include <stdlib.h>

#include "cc/ast.h"
#include "cc/compile/error.h"
#include "cc/compile/type.h"

struct CompileResult analyze_function_signature(
    struct FunctionSignature *const out,
    struct AstFunctionSignature const *const ast,
    struct Compiler *const compiler
) {
    // return type
    struct CompileResult result = analyze_type(&out->return_type, &ast->return_type, compiler);
    if (!result.ok) return result; 

    // parameter types
    out->parameter_count = ast->parameter_count;
    out->parameters = malloc(sizeof *out->parameters * out->parameter_count);

    for (
        usize parameter_index = 0u; 
        parameter_index < ast->parameter_count;
        parameter_index += 1u
    ) {
        struct FunctionParameter *const out_parameter = &out->parameters[parameter_index];
        struct AstFunctionParameter const *const ast_parameter = &ast->parameters[parameter_index];

        out_parameter->name = ast_parameter->identifier.name;
        out_parameter->ast_node_position = ast_parameter->position;

        struct CompileResult result 
            = analyze_type(&out_parameter->type, &ast_parameter->type, compiler);
        if (!result.ok) return result; 
    }

    return compile_ok();
}

