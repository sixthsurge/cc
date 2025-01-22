#include "cc/token.h"

#include "cc/slice.h"
#include "cc/writer.h"

void token_kind_debug(struct Writer *writer, enum TokenKind const *kind) {
    char const *const token_names[] = {
        "<unknown>",
        // misc
        "<eof>",
        ";",
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        ",",
        // operators
        "=",
        "==",
        "+",
        "-",
        "*",
        "/",
        // keywords
        "return",
        "if",
        "else",
        "do",
        "while",
        "for",
        "switch",
        "continue",
        "break",
        "const",
        "void",
        "int",
        "signed",
        "unsigned",
        "long",
        "short",
        "char",
        "float",
        "double",
        // variable tokens
        "<identifier>",
        "<integer>"
    };

    if (*kind < TokenCount) {
        writer_write(writer, token_names[*kind]);
    } else {
        writer_write(writer, "<unknown>");
    }
}

void token_debug(struct Writer *writer, struct Token const *token) {
    switch (token->kind) {
        case TokenIdentifier: {
            writer_write(writer, "<identifier: ");
            writer_write_charslice(writer, token->variant.identifier.name);
            writer_write(writer, ">");
            break;
        }
        case TokenInteger: {
            writer_writef(
                writer, 
                "<integer: %lu, signed=%b, long=%b>", 
                token->variant.integer.value,
                token->variant.integer.is_signed,
                token->variant.integer.is_long
            );
            break;
        }
        default: {
            token_kind_debug(writer, &token->kind);
            break;
        }
    }
}

#define SLICE_ELEMENT_TYPE struct Token
#define SLICE_TYPE TokenSlice 
#define SLICE_FUNCTION_PREFIX tokenslice_
#define SLICE_DEBUG_FN token_debug
#include "cc/template/slice.inl"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX
#undef SLICE_DEBUG_FN

#define VEC_ELEMENT_TYPE struct Token
#define VEC_TYPE TokenVec
#define VEC_SLICE_TYPE TokenSlice
#define VEC_FUNCTION_PREFIX tokenvec_
#define VEC_DEBUG_FN token_debug
#include "cc/template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
#undef VEC_DEBUG_FN 
