#pragma once

#include "arena.h"
#include "ast.h"
#include "token.h"

enum ParseErrorKind {
    ParseErrorUnexpectedToken,
    ParseErrorExpectedPrimaryExpression,
};

struct Parser {
    TokenSlice tokens;
    Token last_token;
    Arena *ast_arena;
};

struct ParseError {
    enum ParseErrorKind kind;
    TokenPosition position;

    union {
        struct {
            TokenKind found;
            TokenKind const *expected;
            usize expected_count;
        } unexpected_token;
        struct {
            char const *required;
        } required_item;
    };
};

struct ParseResult {
    bool ok;
    struct ParseError error;
};

void parser_init(struct Parser *self, Arena *ast_arena, TokenSlice tokens);

struct ParseResult parse_block(struct AstBlock *out, struct Parser *parser);
struct ParseResult parse_statement(struct AstStatement *out, struct Parser *parser);
struct ParseResult parse_variable_declaration(struct AstVariableDeclaration *out, struct Parser *parser);
struct ParseResult parse_return(struct AstReturn *out, struct Parser *parser);
struct ParseResult parse_expression(struct AstExpression *out, struct Parser *parser);
struct ParseResult parse_assignment_expression(struct AstExpression *out, struct Parser *parser);
struct ParseResult parse_additive_expression(struct AstExpression *out, struct Parser *parser);
struct ParseResult parse_multiplicative_expression(struct AstExpression *out, struct Parser *parser);
struct ParseResult parse_primary_expression(struct AstExpression *out, struct Parser *parser);

void parse_error_debug(struct Writer *writer, struct ParseError const *error);
