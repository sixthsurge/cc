#pragma once

#include "arena.h"
#include "ast.h"
#include "token.h"

typedef struct Parser {
    TokenSlice tokens;
    Token last_token;

} Parser;

void parser_init(Parser *self, TokenSlice tokens);

typedef struct ParseResult {
    AstNode node;
    bool success;
} ParseResult;

ParseResult parse_block(Arena *ast_arena, Parser *parser);
ParseResult parse_statement(Arena *ast_arena, Parser *parser);
ParseResult parse_int_declaration(Arena *ast_arena, Parser *parser);
ParseResult parse_return_statement(Arena *ast_arena, Parser *parser);
ParseResult parse_expression(Arena *ast_arena, Parser *parser);
ParseResult parse_assignment_expression(Arena *ast_arena, Parser *parser);
ParseResult parse_additive_expression(Arena *ast_arena, Parser *parser);
ParseResult parse_multiplicative_expression(Arena *ast_arena, Parser *parser);
ParseResult parse_primary_expression(Arena *ast_arena, Parser *parser);


