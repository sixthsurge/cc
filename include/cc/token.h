#pragma once

#include "cc/common.h"
#include "cc/slice.h"

struct Writer;

enum TokenKind {
    TokenUnknown,

    // Misc
    TokenEof,
    TokenSemicolon,
    TokenLeftParen,
    TokenRightParen,
    TokenLeftBrace,
    TokenRightBrace,
    TokenLeftBracket,
    TokenRightBracket,
    TokenComma,

    // Operators
    TokenEquals,
    TokenDoubleEquals,
    TokenPlus,
    TokenMinus,
    TokenAsterisk,
    TokenSlash,
    
    // Keywords
    TokenKeywordReturn,
    TokenKeywordIf,
    TokenKeywordElse,
    TokenKeywordDo,
    TokenKeywordWhile,
    TokenKeywordFor,
    TokenKeywordSwitch,
    TokenKeywordContinue,
    TokenKeywordBreak,
    TokenKeywordConst,
    TokenKeywordVoid,
    TokenKeywordInt,
    TokenKeywordSigned,
    TokenKeywordUnsigned,
    TokenKeywordLong,
    TokenKeywordShort,
    TokenKeywordChar,
    TokenKeywordFloat,
    TokenKeywordDouble,

    // Variable tokens
    TokenIdentifier,
    TokenInteger,

    TokenCount,
};

struct TokenPosition {
    usize line;
    usize character;
};

struct Token {
    enum TokenKind kind;
    struct TokenPosition position;

    union {
        struct {
            struct CharSlice name;
        } identifier;
        struct {
            u64 value;
            bool is_long;
            bool is_signed;
        } integer;
    } variant;
};

void token_kind_debug(struct Writer *writer, enum TokenKind const *kind);
void token_debug(struct Writer *writer, struct Token const *token);

#define SLICE_TYPE TokenSlice 
#define SLICE_ELEMENT_TYPE struct Token
#define SLICE_FUNCTION_PREFIX tokenslice_
#define SLICE_DEBUG_FN token_debug
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX
#undef SLICE_DEBUG_FN

#define VEC_TYPE TokenVec
#define VEC_ELEMENT_TYPE struct Token
#define VEC_SLICE_TYPE TokenSlice
#define VEC_FUNCTION_PREFIX tokenvec_
#define VEC_DEBUG_FN token_debug
#include "template/vec.h"
#undef VEC_TYPE
#undef VEC_ELEMENT_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
#undef VEC_DEBUG_FN
