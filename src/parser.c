#include "parser.h"

#include <stdlib.h>

#include "ast.h"
#include "log.h"
#include "token.h"
#include "type.h"

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

bool parser_accept(Parser *const self, TokenKind type) {
    if (parser_peek(self).kind == type) {
        parser_next(self);
        return true;
    } else {
        return false;
    }
}

void parser_expect(Parser *const self, TokenKind type) {
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

ParseResult parse_block(Arena *const ast_arena, Parser *const parser) {
    PtrVec nodes;
    ptrvec_init(&nodes);

    parser_expect(parser, TokenLeftBrace);

    while (parser_peek(parser).kind != TokenRightBrace) {
        AstNode const node = require(parse_statement(ast_arena, parser));
        ptrvec_push(&nodes, (void *) copy_into_arena(ast_arena, node));
    }

    return ok((AstNode) {
        .kind = AstNodeBlock,
        .block = (AstBlock) {
            .nodes = nodes,
        },
    });
}

ParseResult parse_statement(Arena *const ast_arena, Parser *const parser) {
    AstNode node;

    // awful
    if (parser_peek(parser).kind == TokenKeywordInt) {
        node = require(parse_int_declaration(ast_arena, parser));
    } else if (parser_peek(parser).kind == TokenKeywordReturn) {
        node = require(parse_return_statement(ast_arena, parser));
    } else {
        node = require(parse_expression(ast_arena, parser));
    }

    parser_expect(parser, TokenSemicolon);
    return ok(node);
}

ParseResult parse_int_declaration(Arena *const ast_arena, Parser *const parser) {
    // for now, the only supported declarations are `int <identifier> = <expression>`
    // no other types are supported. you can't even declare without assigning a value
    // this function is awful

    AstDeclaration declaration;

    parser_expect(parser, TokenKeywordInt);
    parser_expect(parser, TokenIdentifier);
    declaration.name = parser->last_token.identifier_name;
    parser_expect(parser, TokenOperatorAssignment);
    AstNode const expression = require(parse_expression(ast_arena, parser));
    declaration.expression = copy_into_arena(ast_arena, expression);

    return ok((AstNode) {
        .kind = AstNodeDeclaration,
        .declaration = declaration,
    });
}

ParseResult parse_return_statement(Arena *const ast_arena, Parser *const parser) {
    parser_expect(parser, TokenKeywordReturn);
    AstNode const expression = require(parse_expression(ast_arena, parser));

    return ok((AstNode) {
        .kind = AstNodeReturn,
        .return_statement = (AstReturn) {
            .expression = copy_into_arena(ast_arena, expression),
        },
    });
}

ParseResult parse_expression(Arena *const ast_arena, Parser *const parser) {
    return parse_assignment_expression(ast_arena, parser);
}

ParseResult parse_assignment_expression(Arena *const ast_arena, Parser *const parser) {
    AstNode const left = require(parse_additive_expression(ast_arena, parser));

    if (parser_accept(parser, TokenOperatorAssignment)) {
        AstNode const right = require(parse_assignment_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeAssignment,
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
            .kind = AstNodeMultiplication,
            .multiplication = (AstMultiplication) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else if (parser_accept(parser, TokenOperatorDiv)) {
        AstNode const right = require(parse_multiplicative_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeDivision,
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
            .kind = AstNodeAddition,
            .addition = (AstAddition) {
                .left = copy_into_arena(ast_arena, left),
                .right = copy_into_arena(ast_arena, right),
            }
        });
    } else if (parser_accept(parser, TokenOperatorSub)) {
        AstNode const right = require(parse_additive_expression(ast_arena, parser));

        return ok((AstNode) {
            .kind = AstNodeSubtraction,
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
            .kind = AstNodeIdentifier,
            .identifier = (AstIdentifier) {
                .name = parser->last_token.identifier_name,
            },
        });
    } else if (parser_accept(parser, TokenInteger)) {
        return ok((AstNode) {
            .kind = AstNodeInteger,
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

