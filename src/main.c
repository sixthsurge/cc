#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "common.h"
#include "hash.h"
#include "lexer.h"
#include "log.h"
#include "parser.h"
#include "read_file_to_string.h"
#include "slice.h"
#include "token.h"
#include "writer.h"

#define ARENA_BLOCK_LEN (1024 * 1024)

i32 main() {
    Writer stdout_writer = file_writer(stdout);

    log_init(
        LOG_LEVEL_TRACE, 
        true,
        false,
        NULL
    );

    char const *const source = read_file_to_string("examples/test.c");

    // Lexical analysis

    log_trace("Lexical analysis");

    TokenVec tokens = tokenize(source);

    puts("Tokens: ");
    tokenvec_debug(&stdout_writer, &tokens);
    puts("\n");

    // Syntax analysis

    log_trace("Syntax analysis");

    Arena ast_arena;
    arena_init(&ast_arena, ARENA_BLOCK_LEN);

    struct Parser parser;
    parser_init(&parser, &ast_arena, tokenvec_slice_whole(&tokens));

    struct AstRoot ast;
    struct ParseResult const result = parse_root(&ast, &parser);

    if (!result.ok) {
        log_error("parsing error %zu:%zu", result.error.position.line, result.error.position.character);
        parse_error_debug(&stdout_writer, &result.error);
        return 1;
    }

    puts("AST:");
    ast_debug_block(&stdout_writer, &ast);
    puts("\n");

    // Compiling

    log_trace("Compiling");

    // Cleanup

    arena_free(&ast_arena);
    tokenvec_free(&tokens);

    return 0;
}

