#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>

#include "slice.h"
#include "token.h"

typedef struct Lexer {
    char const *source_string;

    usize character_index;
    usize line_index;
    usize line_start_index;
} Lexer;

static bool is_whitespace(char const c) {
    return c == ' '
        || c == '\t'
        || c == '\n'
        || c == '\r';
}

static bool is_letter_or_underscore(char const c) {
    return isalpha(c) || c == '_';
}

static bool is_letter_underscore_or_digit(char const c) {
    return is_letter_or_underscore(c) || isdigit(c);
}

static void lexer_init(Lexer *const self, char const *const source_string) {
    self->source_string = source_string;
    self->character_index = 0u;
    self->line_index = 1u;
    self->line_start_index = 0u;
}

static char lexer_next_char(Lexer *const self) {
    char const c = self->source_string[self->character_index];
    self->character_index += 1;

    // update line count
    if (c == '\n') {
        self->line_index += 1;
        self->line_start_index = self->character_index;
    }

    return c;
}

static char lexer_peek_char(Lexer *const self) {
    return self->source_string[self->character_index];
}

static void lexer_skip_whitespace(Lexer *const self) {
    while (is_whitespace(lexer_peek_char(self))) {
        lexer_next_char(self);
    }
}

static CharSlice lexer_next_word(Lexer *const self) {
    char const *const ptr = self->source_string + self->character_index;

    usize len = 0;
    while (is_letter_underscore_or_digit(lexer_peek_char(self))) {
        len++;
        lexer_next_char(self);
    }

    return (CharSlice) { (char *) ptr, len };
}

static Token lexer_next_token(Lexer *const self) {
    lexer_skip_whitespace(self);

    char const c = lexer_next_char(self);

    Token token = (Token) {
        .position = (TokenPosition) {
            .line = self->line_index,
            .character = self->character_index - self->line_start_index,
        }
    };

    switch (c) {
        case '\0': {
            token.kind = TokenEof;
            break;
        }
        case ';': {
            token.kind = TokenSemicolon;
            break;
        }
        case '(': {
            token.kind = TokenLeftParen;
            break;
        }   
        case ')': {
            token.kind = TokenRightParen;
            break;
        }   
        case '{': {
            token.kind = TokenLeftBrace;
            break;
        }
        case '}': {
            token.kind = TokenRightBrace;
            break;
        }
        case '[': {
            token.kind = TokenLeftBracket;
            break;
        }
        case ']': {
            token.kind = TokenRightBracket;
            break;
        }
        case '=': {
            char c_next = lexer_peek_char(self);

            if (c_next == '=') {
                // double = -> equality
                token.kind = TokenOperatorEquality;
                lexer_next_char(self);
            } else {
                // single = -> assignment
                token.kind = TokenOperatorAssignment;
            }
            break;
        }
        case '+': {
            token.kind = TokenOperatorAdd;
            break;
        }
        case '-': {
            token.kind = TokenOperatorSub;
            break;
        }
        case '*': {
            token.kind = TokenOperatorMul;
            break;
        }
        case '/': {
            token.kind = TokenOperatorDiv;
            break;
        }
        default: {
            if (isdigit(c)) {
                // number
                self->character_index -= 1;
                CharSlice const word = lexer_next_word(self);

                token.kind = TokenInteger;
                token.integer_value = atoi(word.ptr);
            } else if (is_letter_or_underscore(c)) {
                // word
                self->character_index -= 1;
                CharSlice const word = lexer_next_word(self);

                if (charslice_eq_cstr(word, "return")) {
                    token.kind = TokenKeywordReturn;
                } else if (charslice_eq_cstr(word, "if")) {
                    token.kind = TokenKeywordIf;
                } else if (charslice_eq_cstr(word, "do")) {
                    token.kind = TokenKeywordDo;
                } else if (charslice_eq_cstr(word, "while")) {
                    token.kind = TokenKeywordWhile;
                } else if (charslice_eq_cstr(word, "for")) {
                    token.kind = TokenKeywordFor;
                } else if (charslice_eq_cstr(word, "switch")) {
                    token.kind = TokenKeywordSwitch;
                } else if (charslice_eq_cstr(word, "continue")) {
                    token.kind = TokenKeywordContinue;
                } else if (charslice_eq_cstr(word, "break")) {
                    token.kind = TokenKeywordBreak;
                } else if (charslice_eq_cstr(word, "const")) {
                    token.kind = TokenKeywordConst;
                } else if (charslice_eq_cstr(word, "void")) {
                    token.kind = TokenKeywordVoid;
                } else if (charslice_eq_cstr(word, "int")) {
                    token.kind = TokenKeywordInt;
                } else if (charslice_eq_cstr(word, "signed")) {
                    token.kind = TokenKeywordSigned;
                } else if (charslice_eq_cstr(word, "unsigned")) {
                    token.kind = TokenKeywordUnsigned;
                } else if (charslice_eq_cstr(word, "long")) {
                    token.kind = TokenKeywordLong;
                } else if (charslice_eq_cstr(word, "short")) {
                    token.kind = TokenKeywordShort;
                } else if (charslice_eq_cstr(word, "char")) {
                    token.kind = TokenKeywordChar;
                } else if (charslice_eq_cstr(word, "float")) {
                    token.kind = TokenKeywordFloat;
                } else if (charslice_eq_cstr(word, "double")) {
                    token.kind = TokenKeywordDouble;
                } else {
                    token.kind = TokenIdentifier;
                    token.identifier_name = word;
                }
            } else {
                token.kind = TokenUnknown;
            }
        }
    }

    return token;
}

TokenVec tokenize(char const *const source_string) {
    Lexer lexer;
    lexer_init(&lexer, source_string);

    TokenVec tokens;
    tokenvec_init(&tokens);

    Token token;
    do {
        token = lexer_next_token(&lexer);
        tokenvec_push(&tokens, token);
    } while (token.kind != TokenEof);

    return tokens;
}

