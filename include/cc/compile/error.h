#pragma once 

#include "cc/ast.h"
#include "cc/slice.h"
#include "cc/token.h"
#include "cc/type.h"
#include "cc/writer.h"

enum CompileErrorKind {
    CompileErrorUnknown,
    CompileErrorNotImplemented,
    CompileErrorUndeclaredIdentifier,
    CompileErrorIncompatibleTypes,
    CompileErrorVariableRedeclaration,
    CompileErrorFunctionRedefinition,
    CompileErrorFunctionSignatureMismatch,
};

struct CompileError {
    enum CompileErrorKind kind;
    struct AstNodePosition position;

    union {
        struct {
            struct CharSlice name;
        } undeclared_identifier;
        struct {
            struct Type first;
            struct Type second;
        } incompatible_types;
        struct {
            struct CharSlice name;
        } variable_redeclaration;
        struct {
            struct CharSlice name;
        } function_redefinition;
        struct {
            struct CharSlice name;
        } function_signature_mismatch;
    };
};

struct CompileResult {
    bool ok;
    struct TokenPosition error_position;
    struct CompileError error;
};

struct CompileResult compile_ok();
struct CompileResult compile_error(struct CompileError error);

void format_compile_error(struct Writer *writer, struct CompileError const *error);

