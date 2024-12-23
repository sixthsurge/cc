#pragma once

#include "common.h"
#include "slice.h"

struct Writer;

typedef enum {
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

    // Operators
    TokenOperatorAssignment,
    TokenOperatorEquality,
    TokenOperatorAdd,
    TokenOperatorSub,
    TokenOperatorMul,
    TokenOperatorDiv,
    
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
} TokenKind;

typedef struct {
    usize line;
    usize character;
} TokenPosition;

typedef struct {
    TokenKind kind;
    TokenPosition position;

    union {
        CharSlice identifier_name;
        i32 integer_value;
    };
} Token;

void token_kind_debug(struct Writer *writer, TokenKind const *kind);
void token_debug(struct Writer *writer, Token const *token);

#define SLICE_TYPE TokenSlice 
#define SLICE_ELEMENT_TYPE Token
#define SLICE_FUNCTION_PREFIX tokenslice_
#define SLICE_DEBUG_FN token_debug
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX
#undef SLICE_DEBUG_FN

#define VEC_TYPE TokenVec
#define VEC_ELEMENT_TYPE Token
#define VEC_SLICE_TYPE TokenSlice
#define VEC_FUNCTION_PREFIX tokenvec_
#define VEC_DEBUG_FN token_debug
#include "template/vec.h"
#undef VEC_TYPE
#undef VEC_ELEMENT_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
#undef VEC_DEBUG_FN
