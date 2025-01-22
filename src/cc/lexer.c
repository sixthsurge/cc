#include "cc/lexer.h"

#include <ctype.h>
#include <stdlib.h>

#include "cc/log.h"
#include "cc/slice.h"
#include "cc/token.h"

struct Lexer {
    char const *source_string;

    usize character_index;
    usize line_index;
    usize line_start_index;
};

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

static bool parse_integer_literal(
    struct CharSlice const word,
    u64 *const value_out,
    bool *const is_long,
    bool *const is_signed
) {
    usize char_index = 0u;

    // detect base

    u64 base = 10u;
    if (word.len >= 2u && word.ptr[0] == '0') {
        if (word.ptr[1] == 'x' || word.ptr[1] == 'X') {
            // hexadecimal
            base = 16u;
            char_index += 2u;
        } else if (word.ptr[1] == 'b' || word.ptr[1] == 'B') {
            // binary 
            base = 2u;
            char_index += 2u;
        } else {
            // octal
            base = 8u;
            char_index += 1u;
        } 
    } 

    // parse value

    u64 value = 0u;

    while (char_index < word.len) {
        char const c = word.ptr[char_index];

        u64 digit_value;
        if ('0' <= c && c <= '9') {
            // decimal digit
            digit_value = (u64) c - (u64) '0';
        } else if (base == 16u && 'a' <= c && c <= 'f') {
            // lowercase hexadecimal digit
            digit_value = (u64) c - (u64) 'a' + 10u;
        } else if (base == 16u && 'A' <= c && c <= 'F') {
            // uppercase hexadecimal digit
            digit_value = (u64) c - (u64) 'A' + 10u;
        } else {
            break;
        }

        if (digit_value >= base) {
            return false;
        }

        value = value * base + digit_value;
        char_index += 1u;
    }

    *value_out = value;
    *is_long = false;
    *is_signed = true;

    // detect unsigned suffix 

    if (char_index == word.len) {
        return true;
    }
    if (word.ptr[char_index] == 'u' || word.ptr[char_index] == 'U') {
        *is_signed = false;
        char_index += 1u;
    }

    // detect long suffix

    if (char_index == word.len) {
        return true;
    }
    if (word.ptr[char_index] == 'l' || word.ptr[char_index] == 'L') {
        *is_long = true;
        char_index += 1u;
    }

    return char_index == word.len;
}

static void lexer_init(struct Lexer *const self, char const *const source_string) {
    self->source_string = source_string;
    self->character_index = 0u;
    self->line_index = 1u;
    self->line_start_index = 0u;
}

static char lexer_next_char(struct Lexer *const self) {
    char const c = self->source_string[self->character_index];
    self->character_index += 1;

    // update line count
    if (c == '\n') {
        self->line_index += 1;
        self->line_start_index = self->character_index;
    }

    return c;
}

static char lexer_peek_char(struct Lexer *const self) {
    return self->source_string[self->character_index];
}

static void lexer_skip_whitespace(struct Lexer *const self) {
    while (is_whitespace(lexer_peek_char(self))) {
        lexer_next_char(self);
    }
}

static struct CharSlice lexer_next_word(struct Lexer *const self) {
    char const *const ptr = self->source_string + self->character_index;

    usize len = 0;
    while (is_letter_underscore_or_digit(lexer_peek_char(self))) {
        len++;
        lexer_next_char(self);
    }

    return (struct CharSlice) { (char *) ptr, len };
}

static struct Token lexer_next_token(struct Lexer *const self) {
    lexer_skip_whitespace(self);

    char const c = lexer_next_char(self);

    struct Token token = (struct Token) {
        .position = (struct TokenPosition) {
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
        case ',': {
            token.kind = TokenComma;
            break;
        }
        case '=': {
            char c_next = lexer_peek_char(self);

            if (c_next == '=') {
                // double = -> equality
                token.kind = TokenDoubleEquals;
                lexer_next_char(self);
            } else {
                // single = -> assignment
                token.kind = TokenEquals;
            }
            break;
        }
        case '+': {
            token.kind = TokenPlus;
            break;
        }
        case '-': {
            token.kind = TokenMinus;
            break;
        }
        case '*': {
            token.kind = TokenAsterisk;
            break;
        }
        case '/': {
            token.kind = TokenSlash;
            break;
        }
        default: {
            if (isdigit(c)) {
                // number
                self->character_index -= 1;
                struct CharSlice const word = lexer_next_word(self);

                bool ok = parse_integer_literal(
                    word,
                    &token.variant.integer.value,
                    &token.variant.integer.is_long,
                    &token.variant.integer.is_signed
                );

                if (ok) {
                    token.kind = TokenInteger;
                } else {
                    token.kind = TokenUnknown;
                }
            } else if (is_letter_or_underscore(c)) {
                // word
                self->character_index -= 1;
                struct CharSlice const word = lexer_next_word(self);

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
                    token.variant.identifier.name = word;
                }
            } else {
                token.kind = TokenUnknown;
            }
        }
    }

    return token;
}

struct TokenVec tokenize(char const *const source_string) {
    struct Lexer lexer;
    lexer_init(&lexer, source_string);

    struct TokenVec tokens;
    tokenvec_init(&tokens);

    struct Token token;
    do {
        token = lexer_next_token(&lexer);
        tokenvec_push(&tokens, token);
    } while (token.kind != TokenEof);

    return tokens;
}

