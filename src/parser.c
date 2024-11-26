#include "parser.h"

#include <stdlib.h>

#include "ast.h"
#include "log.h"
#include "token.h"

AstNode *copy_into_arena(Arena *const arena, AstNode const node) {
    AstNode *const ptr = arena_alloc(arena, sizeof (AstNode));
    *ptr = node;
    return ptr;
}

NORETURN void parser_error(char const *const message) {
    log_info("parser error: %s", message);
    exit(1);
}

void parser_init(Parser *const self, TokenSlice const tokens) {
    self->tokens = tokens;
}

Token parser_peek(Parser *const self) {
    return self->tokens.ptr[0];
}

void parser_next(Parser *const self) {
    if (self->tokens.len > 0) {
        self->last_token = parser_peek(self);
        self->tokens.ptr++;
        self->tokens.len--;
    } else {
        parser_error("expected another token");
    }
}

bool parser_accept(Parser *const self, TokenType type) {
    if (parser_peek(self).type == type) {
        parser_next(self);
        return true;
    } else {
        return false;
    }
}

void parser_expect(Parser *const self, TokenType type) {
    if (!parser_accept(self, type)) {
        parser_error("unexpected token");
    }
}

AstNode require(ParseResult result) {
    if (result.success) {
        return result.node;
    } else {
        parser_error("error");
    }
}

ParseResult ok(AstNode node) {
    return (ParseResult) {
        .node = node,
        .success = true,
    };
}

ParseResult failed() {
    return (ParseResult) {
        .node = {},
        .success = false,
    };
}

ParseResult parse_statement(Arena *const ast_arena, Parser *const parser) {

}

ParseResult parse_expression(Arena *const ast_arena, Parser *const parser) {
    return parse_assignment_expression(ast_arena, parser);
}

ParseResult parse_assignment_expression(Arena *const ast_arena, Parser *const parser) {
    AstNode const left = require(parse_additive_expression(ast_arena, parser));

    if (parser_accept(parser, TokenOperatorAssignment)) {
        AstNode const right = require(parse_assignment_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeKindAssignment,
            .assignment = (AstAssignment) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else {
        return ok(left);
    }
}

ParseResult parse_multiplicative_expression(Arena *ast_arena, Parser *parser) {
    AstNode const left = require(parse_primary_expression(ast_arena, parser));

    if (parser_accept(parser, TokenOperatorMul)) {
        AstNode const right = require(parse_multiplicative_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeKindMultiplication,
            .multiplication = (AstMultiplication) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else if (parser_accept(parser, TokenOperatorDiv)) {
        AstNode const right = require(parse_multiplicative_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeKindDivision,
            .division = (AstDivision) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else {
        return ok(left);
    }
}

ParseResult parse_additive_expression(Arena *const ast_arena, Parser *const parser) {
    AstNode left = require(parse_multiplicative_expression(ast_arena, parser));

    if (parser_accept(parser, TokenOperatorAdd)) {
        AstNode const right = require(parse_additive_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeKindAddition,
            .addition = (AstAddition) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else if (parser_accept(parser, TokenOperatorSub)) {
        AstNode const right = require(parse_additive_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeKindSubtraction,
            .subtraction = (AstSubtraction) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else {
        return ok(left);
    }
}

ParseResult parse_primary_expression(Arena *const ast_arena, Parser *const parser) {
    if (parser_accept(parser, TokenIdentifier)) {
        return ok((AstNode) {
            .kind = AstNodeKindIdentifier,
            .identifier = (AstIdentifier) {
                .name = parser->last_token.identifier_name,
            },
        });
    } else if (parser_accept(parser, TokenInteger)) {
        return ok((AstNode) {
            .kind = AstNodeKindInteger,
            .integer = (AstInteger) {
                .value = parser->last_token.integer_value,
            },
        });
    } else if (parser_accept(parser, TokenLeftParen)) {
        AstNode const expression = require(parse_expression(ast_arena, parser));
        parser_expect(parser, TokenRightParen);

        return ok(expression);
    } else {
        return failed();
    }
}    

