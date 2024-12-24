#include "parser.h"

#include <stdlib.h>

#include "ast.h"
#include "log.h"
#include "token.h"
#include "writer.h"

typedef struct ParseResult (*ExpressionParser)(struct AstExpression *, struct Parser *);

void parser_init(struct Parser *const self, Arena *const ast_arena, TokenSlice const tokens) {
    self->tokens = tokens;
    self->ast_arena = ast_arena;
}

static Token parser_peek(struct Parser const *const self) {
    return self->tokens.ptr[0];
}

static bool parser_next(struct Parser *const self) {
    if (self->tokens.len > 0) {
        self->last_token = parser_peek(self);
        self->tokens.ptr++;
        self->tokens.len--;
        return true;
    } else {
        return false;
    }
}

static bool parser_accept(struct Parser *const self, TokenKind type) {
    if (parser_peek(self).kind == type) {
        parser_next(self);
        return true;
    } else {
        return false;
    }
}

static struct ParseError parser_expected_tokens(
    struct Parser const *const self,
    TokenKind const *expected,
    usize expected_count
) {
    return (struct ParseError) {
        .kind = ParseErrorUnexpectedToken,
        .position = parser_peek(self).position,
        .unexpected_token = {
            .found = parser_peek(self).kind,
            .expected = arena_copy(self->ast_arena, expected, sizeof (TokenKind) * expected_count),
            .expected_count = expected_count,
        },
    };
}

static struct ParseError parser_expected_token(
    struct Parser const *const self,
    TokenKind const expected
) {
    return parser_expected_tokens(self, &expected, 1u);
}

static struct ParseResult parse_ok() {
    return (struct ParseResult) {
        .ok = true,
    };
}

static struct ParseResult parse_error(struct ParseError error) {
    return (struct ParseResult) {
        .ok = false,
        .error = error,
    };
}

#define PARSER_REQUIRE(RESULT) \
    if (!result.ok) {          \
        return result;         \
    }

#define PARSER_EXPECT(PARSER, EXPECTED_TOKEN)                                \
    if (!parser_accept(PARSER, EXPECTED_TOKEN)) {                            \
        return parse_error(parser_expected_token(PARSER, EXPECTED_TOKEN)); \
    }

static struct ParseResult parse_binary_operation(
    struct AstExpression *const out,
    struct Parser *const parser,
    usize op_count,
    enum AstBinaryOpKind const *ops,
    TokenKind const *tokens,
    ExpressionParser const same_precedence,
    ExpressionParser const next_precedence
) {
    struct AstExpression left;
    struct ParseResult const result = next_precedence(&left, parser);
    PARSER_REQUIRE(result);

    for (usize op_index = 0; op_index < op_count; op_index += 1) {
        if (parser_accept(parser, tokens[op_index])) {
            struct AstExpression right;
            struct ParseResult const result = same_precedence(&right, parser);
            PARSER_REQUIRE(result);

            *out = (struct AstExpression) {
                .kind = AstExpressionBinaryOp,
                .binary_op = (struct AstBinaryOp) {
                    .kind = ops[op_index],
                    .left = arena_copy(parser->ast_arena, &left, sizeof left),
                    .right = arena_copy(parser->ast_arena, &right, sizeof right),
                },
            };

            return parse_ok();
        }       
    }

    *out = left;
    return parse_ok();
}

struct ParseResult parse_block(struct AstBlock *const out, struct Parser *const parser) {
    PARSER_EXPECT(parser, TokenLeftBrace);

    struct ArenaVec statements;
    arenavec_init(&statements, parser->ast_arena, sizeof (struct AstStatement));

    while (parser_peek(parser).kind != TokenRightBrace) {
        struct AstStatement statement;
        struct ParseResult const statement_result = parse_statement(&statement, parser);

        if (!statement_result.ok) {
            return statement_result;
        }

        arenavec_push(&statements, (void *) &statement);
    }

    out->statements = statements.data;
    out->statement_count = statements.len;

    return parse_ok();
}

struct ParseResult parse_statement(struct AstStatement *const out, struct Parser *const parser) {
    struct ParseResult result;

    result = parse_return(&out->return_statement, parser);
    if (result.ok) {
        out->kind = AstStatementReturn;
        goto Ok;
    }

    result = parse_variable_declaration(&out->variable_declaration, parser);
    if (result.ok) {
        out->kind = AstStatementVariableDeclaration;
        goto Ok;
    }

    result = parse_expression(&out->expression, parser);
    if (result.ok) {
        out->kind = AstStatementExpression;
        goto Ok;
    }

    return result;

Ok:
    PARSER_EXPECT(parser, TokenSemicolon);

    return parse_ok();
}

struct ParseResult parse_variable_declaration(
    struct AstVariableDeclaration *const out, 
    struct Parser *const parser
) {
    PARSER_EXPECT(parser, TokenKeywordInt);
    PARSER_EXPECT(parser, TokenIdentifier);
    out->identifier.name = parser->last_token.identifier_name;

    if (parser_accept(parser, TokenOperatorAssignment)) {
        struct ParseResult const result = parse_expression(&out->assigned_expression, parser);
        PARSER_REQUIRE(result);
    } 

    return parse_ok();
}

struct ParseResult parse_return(struct AstReturn *const out, struct Parser *const parser) {
    PARSER_EXPECT(parser, TokenKeywordReturn);
    return parse_expression(&out->expression, parser);
}

struct ParseResult parse_expression(struct AstExpression *const out, struct Parser *const parser) {
    return parse_assignment_expression(out, parser);
}

struct ParseResult parse_assignment_expression(struct AstExpression *const out, struct Parser *const parser) {
    enum AstBinaryOpKind const ops[] = { AstBinaryOpAssignment };
    TokenKind const tokens[] = { TokenOperatorAssignment };

    return parse_binary_operation(
        out, 
        parser, 
        1,
        ops, 
        tokens, 
        parse_assignment_expression, 
        parse_additive_expression
    );
}

struct ParseResult parse_additive_expression(struct AstExpression *const out, struct Parser *const parser) {
    enum AstBinaryOpKind const ops[] = { AstBinaryOpAddition, AstBinaryOpSubtraction };
    TokenKind const tokens[] = { TokenOperatorAdd, TokenOperatorSub };

    return parse_binary_operation(
        out, 
        parser, 
        2,
        ops, 
        tokens, 
        parse_additive_expression, 
        parse_multiplicative_expression
    );
}

struct ParseResult parse_multiplicative_expression(struct AstExpression *const out, struct Parser *const parser) {
    enum AstBinaryOpKind const ops[] = { AstBinaryOpMultiplication, AstBinaryOpDivision };
    TokenKind const tokens[] = { TokenOperatorMul, TokenOperatorDiv };

    return parse_binary_operation(
        out, 
        parser, 
        2,
        ops, 
        tokens, 
        parse_multiplicative_expression, 
        parse_primary_expression
    );
}

struct ParseResult parse_primary_expression(struct AstExpression *const out, struct Parser *const parser) {
    switch (parser_peek(parser).kind) {
        case TokenIdentifier: {
            parser_next(parser);
            out->kind = AstExpressionIdentifier;
            out->identifier.name = parser->last_token.identifier_name;
            return parse_ok();
        }
        case TokenInteger: {
            parser_next(parser);
            out->kind = AstExpressionConstant;
            out->constant.value = parser->last_token.integer_value;
            return parse_ok();
        }
        case TokenLeftParen: {
            parser_next(parser);
            struct ParseResult result = parse_expression(out, parser);
            PARSER_REQUIRE(result);
            PARSER_EXPECT(parser, TokenRightParen);
            return parse_ok();
        }
        default: {
            TokenKind const expected[] = { TokenIdentifier, TokenInteger, TokenLeftParen };
            return parse_error(parser_expected_tokens(parser, expected, 3));
        }
    }
}    

void parse_error_debug(struct Writer *writer, struct ParseError const *error) {
    switch (error->kind) {
        case ParseErrorUnexpectedToken: {
            writer_write(writer, "unexpected token: expected ");

            for (usize i = 0u; i < error->unexpected_token.expected_count; i += 1u) {
                writer_write(writer, color_magenta);
                token_kind_debug(writer, &error->unexpected_token.expected[i]);
                writer_write(writer, color_reset);

                if (i < error->unexpected_token.expected_count - 1u) {
                    writer_write(writer, " or ");
                }
            }

            writer_write(writer, ", found ");
            writer_write(writer, color_magenta);
            token_kind_debug(writer, &error->unexpected_token.found);
            writer_write(writer, color_reset);
            writer_write(writer, ".");

            break;
        }
        case ParseErrorExpectedPrimaryExpression: {
            writer_write(writer, "expected primary expression.");
            break;
        }
    }
}

