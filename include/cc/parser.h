#pragma once

#include "cc/arena.h"
#include "cc/ast.h"
#include "cc/token.h"
#include "cc/writer.h"

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
    } variant;
};

struct ParseResult {
    bool ok;
    struct ParseError error;
};

struct ParseResult parse(struct AstRoot *out, struct TokenSlice tokens, struct Arena *ast_arena);
void format_parse_error(struct Writer *writer, struct ParseError const *error);

