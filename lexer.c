#include <ctype.h>  // isalnum, isalpha
#include <stddef.h> // size_t
#include <string.h> // strlen, strncmp

#include "lexer.h" // next_token, struct LexerState
#include "token.h" // TOKEN_*, struct Token

#define TRY_MATCH_STATIC(ls, text, type_if_matches)               \
    if (strncmp(ls->src + ls->cursor, text, strlen(text)) == 0) { \
        struct Token t = {                                        \
            .type = type_if_matches,                              \
            .pos = ls->cursor,                                    \
            .len = strlen(text),                                  \
            .line = ls->curr_line,                                \
            .column = ls->curr_column,                            \
        };                                                        \
        ls->cursor += strlen(text);                               \
        ls->curr_column += strlen(text);                          \
        return t;                                                 \
    }

int is_current_valid_name_start(struct LexerState *ls)
{
    return isalpha(ls->src[ls->cursor])
        || ls->src[ls->cursor] == '_';
}

int is_current_valid_name_tail(struct LexerState *ls)
{
    return isalnum(ls->src[ls->cursor])
        || ls->src[ls->cursor] == '_';
}

#define TRY_MATCH_NAME(ls)                       \
    if (is_current_valid_name_start(ls)) {       \
        size_t pos = ls->cursor;                 \
        int len = 0;                             \
        while (is_current_valid_name_tail(ls)) { \
            len += 1;                            \
            ls->cursor += 1;                     \
        }                                        \
        struct Token t = {                       \
            .type = TOKEN_NAME,                  \
            .pos = pos,                          \
            .len = len,                          \
            .line = ls->curr_line,               \
            .column = ls->curr_column,           \
        };                                       \
        ls->curr_column += len;                  \
        return t;                                \
    }

#define TRY_MATCH_EOF(ls)                \
    if (ls->cursor >= strlen(ls->src)) { \
        return (struct Token){           \
            .type = TOKEN_EOF,           \
            .pos = ls->cursor,           \
            .len = 0,                    \
            .line = ls->curr_line,       \
            .column = ls->curr_column,   \
        };                               \
    }

struct Token next_token(struct LexerState *ls) {
    while (ls->cursor < strlen(ls->src) && isspace(ls->src[ls->cursor])) {
        if (ls->src[ls->cursor] == '\n') {
            ls->curr_line += 1;
            ls->curr_column = 0; // reset
        }
        ls->curr_column += 1;
        ls->cursor += 1;
    }

    TRY_MATCH_STATIC(ls, "class", TOKEN_KW_CLASS);
    TRY_MATCH_STATIC(ls, "else", TOKEN_KW_ELSE);
    TRY_MATCH_STATIC(ls, "function", TOKEN_KW_FUNCTION);
    TRY_MATCH_STATIC(ls, "if", TOKEN_KW_IF);
    TRY_MATCH_STATIC(ls, "import", TOKEN_KW_IMPORT);
    TRY_MATCH_STATIC(ls, "package", TOKEN_KW_PACKAGE);
    TRY_MATCH_STATIC(ls, "var", TOKEN_KW_VAR);
    TRY_MATCH_STATIC(ls, "while", TOKEN_KW_WHILE);

    TRY_MATCH_STATIC(ls, ".", TOKEN_OP_DOT);
    // TODO: Other op...

    TRY_MATCH_NAME(ls);

    TRY_MATCH_EOF(ls);

    return (struct Token){
        .type = TOKEN_UNKNOWN,
        .pos = ls->cursor,
        .len = 0,
        .line = ls->curr_line,
        .column = ls->curr_column,
    };
}
