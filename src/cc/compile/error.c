#include "cc/compile/error.h"

#include "cc/common.h"
#include "cc/type.h"
#include "cc/writer.h"

struct CompileResult compile_ok() {
    return (struct CompileResult) {
        .ok = true,
    };
}

struct CompileResult compile_error(struct CompileError const error) {
    return (struct CompileResult) {
        .ok = false,
        .error = error,
    };
}

void format_compile_error(struct Writer *const writer, struct CompileError const *const error) {
    switch (error->kind) {
        case CompileErrorUnknown: {
            writer_write(writer, "unknown error");
            break;
        }
        case CompileErrorNotImplemented: {
            writer_write(writer, "not implemented");
            break;
        }
        case CompileErrorIncompatibleTypes: {
            writer_write(writer, "incompatible types: ");
            type_debug(writer, &error->incompatible_types.first);
            writer_write(writer, " and ");
            type_debug(writer, &error->incompatible_types.second);
            break;
        }
        case CompileErrorVariableRedeclaration: {
            writer_write(writer, "redeclaration of ");
            writer_write(writer, color_magenta);
            writer_write_charslice(writer, error->variable_redeclaration.name);
            writer_write(writer, color_reset);
            break;
        }
        case CompileErrorFunctionRedefinition: {
            writer_write(writer, "redefinition of ");
            writer_write(writer, color_magenta);
            writer_write_charslice(writer, error->function_redefinition.name);
            writer_write(writer, color_reset);
            break;
        }
        case CompileErrorFunctionSignatureMismatch: {
            writer_write(writer, "mismatched function signature in ");
            writer_write(writer, color_magenta);
            writer_write_charslice(writer, error->function_signature_mismatch.name);
            writer_write(writer, color_reset);
            break;
        }
    }
}
