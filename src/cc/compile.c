#include "cc/compile.h"

#include "cc/compile/compiler.h"
#include "cc/compile/function_table.h"
#include "cc/compile/root.h"
#include "cc/compile/variable_table.h"
#include "cc/writer.h"

struct CompileResult compile(struct Writer *assembly_writer, struct AstRoot *const ast) {
    struct CharVec section_text, section_data;
    charvec_init(&section_text);
    charvec_init(&section_data);

    struct Writer writer_text = charvec_writer(&section_text);
    struct Writer writer_data = charvec_writer(&section_data);

    struct VariableTable global_variable_table;
    variable_table_init(&global_variable_table, NULL);

    struct FunctionTable function_table;
    function_table_init(&function_table);

    struct Compiler compiler = {
        .writer_text = writer_text,
        .writer_data = writer_data,
        .variable_table = &global_variable_table,
        .function_table = &function_table,
    };
    struct CompileResult result = compile_root(&compiler, ast);

    // write assembly
    writer_write(assembly_writer, "global main\n");
    writer_write(assembly_writer, "section .data\n");
    writer_write_charslice(assembly_writer, charvec_slice_whole(&section_data));
    writer_write(assembly_writer, "section .text\n");
    writer_write_charslice(assembly_writer, charvec_slice_whole(&section_text));

    variable_table_free(&global_variable_table);
    function_table_free(&function_table);
    charvec_free(&section_text);
    charvec_free(&section_data);

    return result;
}
