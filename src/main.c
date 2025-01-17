#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "ast.h"
#include "common.h"
#include "compile.h"
#include "lexer.h"
#include "log.h"
#include "parser.h"
#include "read_file_to_string.h"
#include "token.h"
#include "vec.h"
#include "writer.h"

#define ARENA_BLOCK_LEN (1024 * 1024)

i32 main(void) {
    struct Writer stdout_writer = file_writer(stdout);

    log_init(
        LOG_LEVEL_TRACE, 
        true,
        false,
        NULL
    );

    char const *const source = read_file_to_string("input/test.c");

    // Lexical analysis

    log_trace("Lexical analysis");

    struct TokenVec tokens = tokenize(source);

    puts("Tokens: ");
    tokenvec_debug(&stdout_writer, &tokens);
    puts("\n");

    // Syntax analysis

    log_trace("Syntax analysis");

    struct Arena ast_arena;
    arena_init(&ast_arena, ARENA_BLOCK_LEN);

    struct AstRoot ast;
    struct ParseResult const parse_result = parse(
        &ast, 
        tokenvec_slice_whole(&tokens),
        &ast_arena
    );

    if (!parse_result.ok) {
        log_error("parsing error");
        parse_error_debug(&stdout_writer, &parse_result.error);
        return 1;
    }

    puts("AST:");
    ast_debug_root(&stdout_writer, &ast);
    puts("\n");

    // Compiling

    log_trace("Compiling");

    struct CharVec assembly_string;
    charvec_init(&assembly_string);

    struct Writer assembly_writer = charvec_writer(&assembly_string);

    compile_root(&assembly_writer, &ast);

    FILE *const asm_file = fopen("output/test.asm", "w");
    fwrite(assembly_string.data, sizeof (char), assembly_string.len, asm_file);
    fclose(asm_file);

    puts("Assembly:");
    writer_write_charslice(&stdout_writer, charvec_slice_whole(&assembly_string));
    puts("");

    // Assembling 

    log_trace("Assembling");

    system("nasm -f elf64 output/test.asm");

    // Linking 

    log_trace("Linking");

    system("ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o output/test output/test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -L/lib64");

    // Cleanup

    charvec_free(&assembly_string);
    arena_free(&ast_arena);
    tokenvec_free(&tokens);

    return 0;
}

