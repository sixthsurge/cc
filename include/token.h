#pragma once

#include "common.h"
#include "slice.h"

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
} TokenType;

typedef struct {
    usize line;
    usize character;
} TokenPosition;

typedef struct {
    TokenType type;
    TokenPosition position;

    union {
        CharSlice identifier_name;
        i32 integer_value;
    };
} Token;

char const *token_type_debug(TokenType);

#define SLICE_ELEMENT_TYPE Token
#define SLICE_TYPE TokenSlice 
#define SLICE_FUNCTION_PREFIX tokenslice_
#include "template/slice.h"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

#define VEC_ELEMENT_TYPE Token
#define VEC_TYPE TokenVec
#define VEC_SLICE_TYPE TokenSlice
#define VEC_FUNCTION_PREFIX tokenvec_
#include "template/vec.h"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX
