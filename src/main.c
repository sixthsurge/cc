#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "common.h"
#include "lexer.h"
#include "log.h"
#include "parser.h"
#include "read_file_to_string.h"
#include "token.h"

#define ARENA_BLOCK_LEN (1024 * 1024)

i32 main() {
    log_init(
        LOG_LEVEL_TRACE, 
        true,
        false,
        NULL
    );

    char const *const source = read_file_to_string("examples/test.c");

    // Lexing

    log_trace("lexing");

    puts("Tokens:");
    TokenVec tokens = tokenize(source);
    for (usize i = 0u; i < tokens.len; i += 1) {
        printf("%s ", token_type_debug(tokenvec_at(&tokens, i)->type));
    }
    puts("\n");

    // Parsing

    log_trace("parsing");

    Arena ast_arena;
    arena_init(&ast_arena, ARENA_BLOCK_LEN);

    Parser parser;
    parser_init(&parser, tokenvec_slice_whole(&tokens));

    ParseResult const result = parse_expression(&ast_arena, &parser);

    puts("AST:");
    ast_debug(stdout, &result.node);
    puts("\n");

    log_trace("cleaning up");

    arena_free(&ast_arena);
    tokenvec_free(&tokens);

    return 0;
}
