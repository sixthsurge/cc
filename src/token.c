#include "token.h"

char const *token_type_debug(TokenType const token_type) {
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

    if (token_type < TokenCount) {
        return token_names[token_type];
    } else {
        return "<unknown>";
    }
}

#define SLICE_ELEMENT_TYPE Token
#define SLICE_TYPE TokenSlice 
#define SLICE_FUNCTION_PREFIX tokenslice_
#include "template/slice.inl"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

#define VEC_ELEMENT_TYPE Token
#define VEC_TYPE TokenVec
#define VEC_SLICE_TYPE TokenSlice
#define VEC_FUNCTION_PREFIX tokenvec_
#include "template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
