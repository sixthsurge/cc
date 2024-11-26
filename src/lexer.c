#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>

#include "slice.h"
#include "token.h"

bool is_whitespace(char c);
bool is_letter_or_underscore(char c);
bool is_letter_underscore_or_digit(char c);

typedef struct {
    char const *source_string;

    usize character_index;
    usize line_index;
    usize line_start_index;
} Lexer;

void lexer_init(Lexer *const self, char const *const source_string) {
    self->source_string = source_string;
    self->character_index = 0u;
    self->line_index = 0u;
    self->line_start_index = 0u;
}

char lexer_next_char(Lexer *const self) {
    char const c = self->source_string[self->character_index];
    self->character_index += 1;

    // update line count
    if (c == '\n') {
        self->line_index = 0u;
        self->line_start_index = self->character_index;
    }

    return c;
}

char lexer_peek_char(Lexer *const self) {
    return self->source_string[self->character_index];
}

void lexer_skip_whitespace(Lexer *const self) {
    while (is_whitespace(lexer_peek_char(self))) {
        lexer_next_char(self);
    }
}

CharSlice lexer_next_word(Lexer *const self) {
    char const *const ptr = self->source_string + self->character_index;

    usize len = 0;
    while (is_letter_underscore_or_digit(lexer_peek_char(self))) {
        len++;
        lexer_next_char(self);
    }

    return (CharSlice) { (char *) ptr, len };
}

Token lexer_next_token(Lexer *const self) {
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
            token.type = TokenEof;
            break;
        }
        case ';': {
            token.type = TokenSemicolon;
            break;
        }
        case '(': {
            token.type = TokenLeftParen;
            break;
        }   
        case ')': {
            token.type = TokenRightParen;
            break;
        }   
        case '{': {
            token.type = TokenLeftBrace;
            break;
        }
        case '}': {
            token.type = TokenRightBrace;
            break;
        }
        case '[': {
            token.type = TokenLeftBracket;
            break;
        }
        case ']': {
            token.type = TokenRightBracket;
            break;
        }
        case '=': {
            char c_next = lexer_peek_char(self);

            if (c_next == '=') {
                // double = -> equality
                token.type = TokenOperatorEquality;
                lexer_next_char(self);
            } else {
                // single = -> assignment
                token.type = TokenOperatorAssignment;
            }
            break;
        }
        case '+': {
            token.type = TokenOperatorAdd;
            break;
        }
        case '-': {
            token.type = TokenOperatorSub;
            break;
        }
        case '*': {
            token.type = TokenOperatorMul;
            break;
        }
        case '/': {
            token.type = TokenOperatorDiv;
            break;
        }
        default: {
            if (isdigit(c)) {
                // number
                self->character_index -= 1;
                CharSlice const word = lexer_next_word(self);

                token.type = TokenInteger;
                token.integer_value = atoi(word.ptr);
            } else if (is_letter_or_underscore(c)) {
                // word
                self->character_index -= 1;
                CharSlice const word = lexer_next_word(self);

                if (charslice_eq_cstr(word, "return")) {
                    token.type = TokenKeywordReturn;
                } else if (charslice_eq_cstr(word, "if")) {
                    token.type = TokenKeywordIf;
                } else if (charslice_eq_cstr(word, "do")) {
                    token.type = TokenKeywordDo;
                } else if (charslice_eq_cstr(word, "while")) {
                    token.type = TokenKeywordWhile;
                } else if (charslice_eq_cstr(word, "for")) {
                    token.type = TokenKeywordFor;
                } else if (charslice_eq_cstr(word, "switch")) {
                    token.type = TokenKeywordSwitch;
                } else if (charslice_eq_cstr(word, "continue")) {
                    token.type = TokenKeywordContinue;
                } else if (charslice_eq_cstr(word, "break")) {
                    token.type = TokenKeywordBreak;
                } else if (charslice_eq_cstr(word, "const")) {
                    token.type = TokenKeywordConst;
                } else if (charslice_eq_cstr(word, "void")) {
                    token.type = TokenKeywordVoid;
                } else if (charslice_eq_cstr(word, "int")) {
                    token.type = TokenKeywordInt;
                } else if (charslice_eq_cstr(word, "signed")) {
                    token.type = TokenKeywordSigned;
                } else if (charslice_eq_cstr(word, "unsigned")) {
                    token.type = TokenKeywordUnsigned;
                } else if (charslice_eq_cstr(word, "long")) {
                    token.type = TokenKeywordLong;
                } else if (charslice_eq_cstr(word, "short")) {
                    token.type = TokenKeywordShort;
                } else if (charslice_eq_cstr(word, "char")) {
                    token.type = TokenKeywordChar;
                } else if (charslice_eq_cstr(word, "float")) {
                    token.type = TokenKeywordFloat;
                } else if (charslice_eq_cstr(word, "double")) {
                    token.type = TokenKeywordDouble;
                } else {
                    token.type = TokenIdentifier;
                    token.identifier_name = word;
                }
            } else {
                token.type = TokenUnknown;
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
    } while (token.type != TokenEof);

    return tokens;
}

bool is_whitespace(char const c) {
    return c == ' '
        || c == '\t'
        || c == '\n'
        || c == '\r';
}

bool is_letter_or_underscore(char const c) {
    return isalpha(c) || c == '_';
}

bool is_letter_underscore_or_digit(char const c) {
    return is_letter_or_underscore(c) || isdigit(c);
}
