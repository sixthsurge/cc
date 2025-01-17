#pragma once

#include "arena.h"
#include "ast.h"
#include "token.h"
#include "writer.h"

enum ParseErrorKind {
    ParseErrorUnknown,
    ParseErrorJoin,          // {ParseError} or {ParseError}
    ParseErrorExpectedToken, // expected {TokenKind}, got {TokenKind}
};

struct ParseError {
    enum ParseErrorKind kind;

    union {
        struct {
            struct ParseError const *left;
            struct ParseError const *right;
        } join;
        struct {
            struct TokenPosition position;
            enum TokenKind expected;
            enum TokenKind got;
        } expected_token;
    };
};

struct ParseResult {
    bool ok;
    struct ParseError error;
};

struct ParseResult parse(struct AstRoot *out, struct TokenSlice tokens, struct Arena *ast_arena);
void parse_error_debug(struct Writer *writer, struct ParseError const *error);

