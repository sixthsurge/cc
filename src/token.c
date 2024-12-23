#include "token.h"

#include "slice.h"
#include "writer.h"

void token_kind_debug(struct Writer *writer, TokenKind const *kind) {
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

void token_debug(struct Writer *writer, Token const *token) {
    switch (token->kind) {
        case TokenIdentifier: {
            writer_write(writer, "<identifier: ");
            writer_write_charslice(writer, token->identifier_name);
            writer_write(writer, ">");
            break;
        }
        case TokenInteger: {
            writer_write(writer, "<integer: ");
            writer_writef(writer, "%d", token->integer_value);
            writer_write(writer, ">");
            break;
        }
        default: {
            token_kind_debug(writer, &token->kind);
            break;
        }
    }
}

#define SLICE_ELEMENT_TYPE Token
#define SLICE_TYPE TokenSlice 
#define SLICE_FUNCTION_PREFIX tokenslice_
#define SLICE_DEBUG_FN token_debug
#include "template/slice.inl"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX
#undef SLICE_DEBUG_FN

#define VEC_ELEMENT_TYPE Token
#define VEC_TYPE TokenVec
#define VEC_SLICE_TYPE TokenSlice
#define VEC_FUNCTION_PREFIX tokenvec_
#define VEC_DEBUG_FN token_debug
#include "template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
#undef VEC_DEBUG_FN 
