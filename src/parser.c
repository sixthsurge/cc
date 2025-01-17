#include "parser.h"

#include <stdarg.h>
#include <stdlib.h>

#include "ast.h"
#include "log.h"
#include "token.h"
#include "vec.h"

#define PARSER_SUCCEED_ON(PARSER, RESULT)               \
    if ((RESULT).ok) {                              \
        return parser_success(PARSER);              \
    }                                               \

#define PARSER_FAIL_ON(PARSER, RESULT)                  \
    do {                                                \
        struct ParseResult const result = (RESULT);     \
        if (!result.ok) {                               \
            return parser_fail(PARSER, result.error);   \
        }                                               \
    } while (false);

struct Parser {
    struct UsizeVec position_stack;
    struct TokenSlice tokens;
    struct Arena *ast_arena;
    struct Token last_token;
};

typedef struct ParseResult (*ExpressionParseFn)(struct AstExpression *, struct Parser *);

static void parser_init(
    struct Parser *const self, 
    struct TokenSlice const tokens, 
    struct Arena *const ast_arena
) {
    self->tokens = tokens;
    self->ast_arena = ast_arena;
    self->last_token = (struct Token) { .kind = TokenUnknown };

    usizevec_init(&self->position_stack);
    usizevec_push(&self->position_stack, 0u);
}

static void parser_free(struct Parser *const self) {
    usizevec_free(&self->position_stack);
}

static struct Token parser_peek(struct Parser const *const self) {
    usize const position = *usizevec_peek_back(&self->position_stack);

    if (position < self->tokens.len) {
        return self->tokens.ptr[position];
    } else {
        return (struct Token) { .kind = TokenUnknown };
    }
}

static struct Token parser_next(struct Parser *const self) {
    struct Token const next = parser_peek(self);
    self->last_token = next;
    *usizevec_peek_back(&self->position_stack) += 1u;
    return next;
}

// push current token position onto the stack 
static void parser_push_position(struct Parser *const self) {
    usize const position = *usizevec_peek_back(&self->position_stack);
    usizevec_push(
        &self->position_stack, 
        position
    );
}

// accept token position and return OK result
static struct ParseResult parser_success(struct Parser *const self) {
    usize const new_position = usizevec_pop_back(&self->position_stack);
    *usizevec_peek_back(&self->position_stack) = new_position;

    return (struct ParseResult) {
        .ok = true,
    };
}

// reject token position and return error result
static struct ParseResult parser_fail(struct Parser *const self, struct ParseError const error) {
    usizevec_pop_back(&self->position_stack);

    return (struct ParseResult) {
        .ok = false,
        .error = error,
    };
}

static bool parser_accept(
    struct Parser *const self,
    enum TokenKind const token
) {
    struct Token const next = parser_peek(self);

    if (next.kind == token) {
        parser_next(self);
        return true;
    } else {
        return false;
    }
}
        
static struct ParseResult parser_expect(
    struct Parser *const self,
    enum TokenKind const token
) {
    if (parser_accept(self, token)) {
        return (struct ParseResult) {
            .ok = true,
        };
    } else {
        struct Token const next = parser_peek(self);
        struct ParseError const error = {
            .kind = ParseErrorExpectedToken,
            .expected_token = {
                .position = next.position,
                .got = next.kind,
                .expected = token,
            },
        };

        return (struct ParseResult) {
            .ok = false,
            .error = error,
        };
    }
}

static struct ParseError join_parse_errors(
    struct Arena *const arena,
    usize const error_count,
    ...
) {
    if (error_count < 2u) {
        log_error("join_parse_errors called with fewer than 2 errors");
        exit(1);
    } 

    va_list errors;
    va_start(errors, error_count);

    struct ParseError left  = va_arg(errors, struct ParseError);
    struct ParseError right = va_arg(errors, struct ParseError);

    /*
    for (usize error_index = 2u; error_index < error_count; error_index += 1) {
        left = (struct ParseError) {
            .kind = ParseErrorJoin,
            .join = {
                .left = arena_copy(arena, &left, sizeof left),
                .right = arena_copy(arena, &right, sizeof right),
            },
        };
        right = va_arg(errors, struct ParseError);
    }
    */

    va_end(errors);

    return (struct ParseError) {
        .kind = ParseErrorJoin,
        .join = {
            .left = arena_copy(arena, &left, sizeof left),
            .right = arena_copy(arena, &right, sizeof right),
        },
    };
}

// generic recursive descent parser for binary operations
static struct ParseResult parse_binary_operation(
    struct AstExpression *const out,
    struct Parser *const parser,
    // number of different operators
    usize op_count,
    // binary operations corresponding to each operator
    enum AstBinaryOpKind const *ops,
    // tokens corresponding to each operator
    enum TokenKind const *tokens,
    // parse function with the same precedence
    ExpressionParseFn const same_precedence,
    // parse function with the next precedence down
    ExpressionParseFn const next_precedence
) {
    parser_push_position(parser);

    struct AstExpression left;
    PARSER_FAIL_ON(parser, next_precedence(&left, parser));

    for (usize op_index = 0; op_index < op_count; op_index += 1) {
        if (parser_accept(parser, tokens[op_index])) {
            struct AstExpression right;
            PARSER_FAIL_ON(parser, same_precedence(&right, parser));

            *out = (struct AstExpression) {
                .kind = AstExpressionBinaryOp,
                .binary_op = (struct AstBinaryOp) {
                    .kind = ops[op_index],
                    .left = arena_copy(parser->ast_arena, &left, sizeof left),
                    .right = arena_copy(parser->ast_arena, &right, sizeof right),
                },
            };

            return parser_success(parser);
        }       
    }

    *out = left;
    return parser_success(parser);
}

// ---------------------
//   parsing functions
// ---------------------

static struct ParseResult parse_expression(
    struct AstExpression *const out, 
    struct Parser *const parser
);

// identifier = `identifier`
static struct ParseResult parse_identifier(
    struct AstIdentifier *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    PARSER_FAIL_ON(
        parser, 
        parser_expect(parser, TokenIdentifier)
    );

    out->name = parser->last_token.identifier_name;
    return parser_success(parser);
}

// constant = `integer`
static struct ParseResult parse_constant(
    struct AstConstant *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    PARSER_FAIL_ON(
        parser, 
        parser_expect(parser, TokenInteger)
    );

    out->value = parser->last_token.integer_value;
    return parser_success(parser);
}

// bracketed_expression = `(` expression `)`
static struct ParseResult parse_bracketed_expression(
    struct AstExpression *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenLeftParen)
    );

    PARSER_FAIL_ON(
        parser,
        parse_expression(out, parser)
    );

    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenRightParen)
    );

    return parser_success(parser);
}

// factor = identifier | constant | bracketed_expression
static struct ParseResult parse_factor(
    struct AstExpression *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    struct ParseResult const identifier_result = parse_identifier(&out->identifier, parser);
    if (identifier_result.ok) {
        out->kind = AstExpressionIdentifier;
        return parser_success(parser);
    }

    struct ParseResult const constant_result = parse_constant(&out->constant, parser);
    if (constant_result.ok) {
        out->kind = AstExpressionConstant;
        return parser_success(parser);
    }

    struct ParseResult const bracketed_expression_result = parse_bracketed_expression(out, parser);
    if (bracketed_expression_result.ok) {
        return parser_success(parser);
    }

    return parser_fail(
        parser,
        join_parse_errors(
            parser->ast_arena, 
            3u, 
            identifier_result.error,
            constant_result.error,
            bracketed_expression_result.error
        )
    );
}

// product = factor `*` factor | factor
static struct ParseResult parse_product(
    struct AstExpression *const out, 
    struct Parser *const parser
) {
    enum AstBinaryOpKind const ops[] = { AstBinaryOpMultiplication, AstBinaryOpDivision };
    enum TokenKind const tokens[] = { TokenOperatorMul, TokenOperatorDiv };

    return parse_binary_operation(
        out, 
        parser, 
        2u,
        ops, 
        tokens, 
        parse_product, 
        parse_factor
    );
}

// sum = product `+` product | product
static struct ParseResult parse_sum(
    struct AstExpression *const out, 
    struct Parser *const parser
) {
    enum AstBinaryOpKind const ops[] = { AstBinaryOpAddition, AstBinaryOpSubtraction };
    enum TokenKind const tokens[] = { TokenOperatorAdd, TokenOperatorSub };

    return parse_binary_operation(
        out, 
        parser, 
        2u,
        ops, 
        tokens, 
        parse_sum, 
        parse_product
    );
}

// assignee = identifier
static struct ParseResult parse_assignee(
    struct AstAssignee *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    PARSER_FAIL_ON(
        parser, 
        parse_identifier(&out->identifier, parser)
    );

    return parser_success(parser);
}

// assignment = assignee `=` expression
static struct ParseResult parse_assignment(
    struct AstExpression *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // assignee
    struct AstAssignee assignee;
    PARSER_FAIL_ON(
        parser,
        parse_assignee(&assignee, parser)
    );

    // assignment operator
    PARSER_FAIL_ON( 
        parser,
        parser_expect(parser, TokenOperatorAssignment)
    );

    // assigned expression
    struct AstExpression assigned_expression;
    PARSER_FAIL_ON(
        parser,
        parse_expression(&assigned_expression, parser)
    );

    out->kind = AstExpressionAssignment;
    out->assignment = (struct AstAssignment) {
        .assignee = assignee,
        .assigned_expression = arena_copy(
            parser->ast_arena, 
            &assigned_expression, 
            sizeof assigned_expression
        ),
    };

    return parser_success(parser);
}

// expression = assignment | sum
static struct ParseResult parse_expression(
    struct AstExpression *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    struct ParseResult const assignment_result = parse_assignment(out, parser);
    PARSER_SUCCEED_ON(parser, assignment_result)

    struct ParseResult const sum_result = parse_sum(out, parser);
    PARSER_SUCCEED_ON(parser, sum_result)

    return parser_fail(
        parser,
        join_parse_errors(
            parser->ast_arena, 
            2u,
            sum_result.error,
            assignment_result.error
        )
    );
}

// type = `int`
static struct ParseResult parse_type(
    struct AstType *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    PARSER_FAIL_ON(
        parser, 
        parser_expect(parser, TokenKeywordInt)
    )

    out->kind = AstTypeIntS32;
    return parser_success(parser);
}

// variable_declaration = type identifier `;` | type identifier `=` expression `;`
static struct ParseResult parse_variable_declaration(
    struct AstVariableDeclaration *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // type
    PARSER_FAIL_ON(
        parser,
        parse_type(&out->type, parser)
    )

    // identifier
    PARSER_FAIL_ON(
        parser,
        parse_identifier(&out->identifier, parser)
    )

    // assigned expression
    if (parser_accept(parser, TokenOperatorAssignment)) {
        out->has_assigned_expression = true;

        PARSER_FAIL_ON(
            parser,
            parse_expression(&out->assigned_expression, parser)
        )
    } else {
        out->has_assigned_expression = false;
    }

    // semicolon
    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenSemicolon)
    )

    return parser_success(parser);
}

// return = `return` `;` | `return` `expression` `;`
static struct ParseResult parse_return(
    struct AstReturn *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // return keyword
    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenKeywordReturn)
    )

    if (parser_accept(parser, TokenSemicolon)) {
        // no returned expression
        out->has_returned_expression = false;
    } else {
        // returned expression
        out->has_returned_expression = true;

        PARSER_FAIL_ON(
            parser,
            parse_expression(&out->returned_expression, parser)
        )

        PARSER_FAIL_ON(
            parser,
            parser_expect(parser, TokenSemicolon)
        )
    }

    return parser_success(parser);
}

// statement = return | variable_declaration | expression `;`
static struct ParseResult parse_statement(
    struct AstStatement *const out,
    struct Parser *const parser
) {
    parser_push_position(parser);

    // return statement
    struct ParseResult const return_result = parse_return(&out->return_statement, parser);
    if (return_result.ok) {
        out->kind = AstStatementReturn;
        return parser_success(parser);
    }

    // variable declaration
    struct ParseResult const variable_declaration_result = parse_variable_declaration(&out->variable_declaration, parser);
    if (variable_declaration_result.ok) {
        out->kind = AstStatementVariableDeclaration;
        return parser_success(parser);
    }

    // expression statement
    struct ParseResult const expression_result = parse_expression(&out->expression, parser);
    if (expression_result.ok) {
        // semicolon 
        PARSER_FAIL_ON(
            parser,
            parser_expect(parser, TokenSemicolon)
        )

        out->kind = AstStatementExpression;
        return parser_success(parser);
    }

    return parser_fail(
        parser,
        join_parse_errors(
            parser->ast_arena, 
            3u,
            expression_result.error,
            variable_declaration_result.error,
            expression_result.error
        )
    );
}

// statement_list = e | statement | statement_list statement
// block = `{` statement_list `}`
static struct ParseResult parse_block(struct AstBlock *const out, struct Parser *const parser) {
    parser_push_position(parser);

    // opening brace
    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenLeftBrace)
    )

    struct ArenaVec statements;
    arenavec_init(&statements, parser->ast_arena, sizeof (struct AstStatement));

    while (!parser_accept(parser, TokenRightBrace)) {
        struct AstStatement statement;
        PARSER_FAIL_ON(
            parser,
            parse_statement(&statement, parser)
        )

        arenavec_push(&statements, &statement);
    }

    out->statements = statements.data;
    out->statement_count = statements.len;

    return parser_success(parser);
}

// parameter = type identifier
static struct ParseResult parse_function_parameter(
    struct AstFunctionParameter *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // type 
    PARSER_FAIL_ON(
        parser,
        parse_type(&out->type, parser)
    )

    // identifier
    struct ParseResult const identifier_result = parse_identifier(&out->identifier, parser);
    out->has_identifier = identifier_result.ok;

    return parser_success(parser);
}

// function_signature = type identifier `(` parameter_list `)`
static struct ParseResult parse_function_signature(
    struct AstFunctionSignature *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // return type 
    PARSER_FAIL_ON(
        parser, 
        parse_type(&out->return_type, parser)
    );

    // identifier
    PARSER_FAIL_ON(
        parser, 
        parse_identifier(&out->identifier, parser)
    );

    // `(`
    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenLeftParen)
    )

    if (parser_accept(parser, TokenRightParen)) {
        // no parameters
        out->parameters = NULL;
        out->parameter_count = 0u;
        return parser_success(parser);
    }

    // parameter list
    struct ArenaVec parameters;
    arenavec_init(&parameters, parser->ast_arena, sizeof (struct AstFunctionParameter));

    do {
        struct AstFunctionParameter parameter;
        PARSER_FAIL_ON(
            parser,
            parse_function_parameter(&parameter, parser)
        )

        arenavec_push(&parameters, &parameter);
    } while (parser_accept(parser, TokenComma));

    // `)`
    PARSER_FAIL_ON(
        parser,
        parser_expect(parser, TokenRightParen)
    )

    out->parameters = parameters.data;
    out->parameter_count = parameters.len;

    return parser_success(parser);
}

// function_definition = function_signature block
static struct ParseResult parse_function_definition(
    struct AstFunctionDefinition *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // signature 
    PARSER_FAIL_ON(
        parser, 
        parse_function_signature(&out->signature, parser)
    )
    
    // body
    PARSER_FAIL_ON(
        parser, 
        parse_block(&out->body, parser)
    )

    return parser_success(parser);
}

// top_level_item = function_definition

static struct ParseResult parse_top_level_item(
    struct AstTopLevelItem *const out, 
    struct Parser *const parser
) {
    parser_push_position(parser);

    // function definition
    struct ParseResult const function_definition_result 
        = parse_function_definition(&out->function_definition, parser);
    if (function_definition_result.ok) {
        out->kind = AstTopLevelItemFunctionDefinition;
        return parser_success(parser);
    }

    return parser_fail(parser, function_definition_result.error);
}

// root = top_level_item | root top_level_item
static struct ParseResult parse_root(struct AstRoot *const out, struct Parser *const parser) {
    parser_push_position(parser);

    struct ArenaVec top_level_items;
    arenavec_init(&top_level_items, parser->ast_arena, sizeof (struct AstTopLevelItem));

    while (!parser_accept(parser, TokenEof)) {
        struct AstTopLevelItem top_level_item;
        PARSER_FAIL_ON(
            parser, 
            parse_top_level_item(&top_level_item, parser)
        )
        arenavec_push(&top_level_items, &top_level_item);
    }

    out->items = top_level_items.data;
    out->item_count = top_level_items.len;

    return parser_success(parser);
}

struct ParseResult parse(struct AstRoot *out, struct TokenSlice tokens, struct Arena *ast_arena) {
    struct Parser parser;
    parser_init(&parser, tokens, ast_arena);
    struct ParseResult const result = parse_root(out, &parser);
    parser_free(&parser);

    return result;
}

void parse_error_debug(
    struct Writer *const writer, 
    struct ParseError const *const error
) {
    switch (error->kind) {
        case ParseErrorExpectedToken: {
            writer_writef(
                writer, 
                "[%zu:%zu] expected ", 
                error->expected_token.position.line, 
                error->expected_token.position.character
            );
            writer_write(writer, color_magenta);
            token_kind_debug(writer, &error->expected_token.expected);
            writer_write(writer, color_reset);

            writer_write(writer, ", got ");
            writer_write(writer, color_magenta);
            token_kind_debug(writer, &error->expected_token.got);
            writer_write(writer, color_reset);

            break;
        }

        case ParseErrorJoin: {
            parse_error_debug(writer, error->join.left);
            writer_writef(writer, " %sOR%s ", color_green, color_reset);
            parse_error_debug(writer, error->join.right);
            break;
        }
        default: {
            writer_writef(writer, "unknown error");
        }
    }
}

