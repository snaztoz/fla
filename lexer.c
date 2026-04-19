#include <ctype.h>  // isalnum, isalpha, isdigit
#include <stddef.h> // size_t
#include <string.h> // strlen, strncmp

#include "lexer.h" // next_token, struct LexerState
#include "token.h" // TOKEN_*, struct Token

char current(struct LexerState *ls)
{
    return ls->src[ls->cursor];
}

#define SKIP_WHITESPACES(ls)                                    \
    while (ls->cursor < strlen(ls->src)                         \
            && isspace(current(ls)))                            \
    {                                                           \
        if (current(ls) == '\n')                                \
        {                                                       \
            ls->curr_line += 1;                                 \
            ls->curr_column = 0;                                \
        }                                                       \
        ls->curr_column += 1;                                   \
        ls->cursor += 1;                                        \
    }

#define TRY_MATCH_STATIC(ls, text, type_if_matches)             \
    if (strncmp(ls->src + ls->cursor, text, strlen(text)) == 0) \
    {                                                           \
        struct Token t = {                                      \
            .type = type_if_matches,                            \
            .pos = ls->cursor,                                  \
            .len = strlen(text),                                \
            .line = ls->curr_line,                              \
            .column = ls->curr_column,                          \
        };                                                      \
        ls->cursor += strlen(text);                             \
        ls->curr_column += strlen(text);                        \
        return t;                                               \
    }

int is_current_valid_name_start(struct LexerState *ls)
{
    return isalpha(current(ls)) || current(ls) == '_';
}

int is_current_valid_name_tail(struct LexerState *ls)
{
    return isalnum(current(ls)) || current(ls) == '_';
}

#define TRY_MATCH_NAME(ls)                                      \
    if (is_current_valid_name_start(ls))                        \
    {                                                           \
        size_t pos = ls->cursor;                                \
        ls->cursor += 1;                                        \
        int len = 1;                                            \
        while (is_current_valid_name_tail(ls))                  \
        {                                                       \
            len += 1;                                           \
            ls->cursor += 1;                                    \
        }                                                       \
        struct Token t = {                                      \
            .type = TOKEN_NAME,                                 \
            .pos = pos,                                         \
            .len = len,                                         \
            .line = ls->curr_line,                              \
            .column = ls->curr_column,                          \
        };                                                      \
        ls->curr_column += len;                                 \
        return t;                                               \
    }

#define TRY_MATCH_NUMBER(ls)                                    \
    if (isdigit(current(ls)))                                   \
    {                                                           \
        size_t pos = ls->cursor;                                \
        ls->cursor += 1;                                        \
        int len = 1;                                            \
        while (isdigit(current(ls)))                            \
        {                                                       \
            len += 1;                                           \
            ls->cursor += 1;                                    \
        }                                                       \
        struct Token t = {                                      \
            .type = TOKEN_NUMBER,                               \
            .pos = pos,                                         \
            .len = len,                                         \
            .line = ls->curr_line,                              \
            .column = ls->curr_column,                          \
        };                                                      \
        ls->curr_column += len;                                 \
        return t;                                               \
    }

#define TRY_MATCH_EOF(ls)                                       \
    if (ls->cursor >= strlen(ls->src))                          \
    {                                                           \
        return (struct Token) {                                 \
            .type = TOKEN_EOF,                                  \
            .pos = ls->cursor,                                  \
            .len = 0,                                           \
            .line = ls->curr_line,                              \
            .column = ls->curr_column,                          \
        };                                                      \
    }

struct Token next_token(struct LexerState *ls)
{
    SKIP_WHITESPACES(ls);

    TRY_MATCH_STATIC(ls, "class", TOKEN_KW_CLASS);
    TRY_MATCH_STATIC(ls, "else", TOKEN_KW_ELSE);
    TRY_MATCH_STATIC(ls, "function", TOKEN_KW_FUNCTION);
    TRY_MATCH_STATIC(ls, "if", TOKEN_KW_IF);
    TRY_MATCH_STATIC(ls, "import", TOKEN_KW_IMPORT);
    TRY_MATCH_STATIC(ls, "package", TOKEN_KW_PACKAGE);
    TRY_MATCH_STATIC(ls, "var", TOKEN_KW_VAR);
    TRY_MATCH_STATIC(ls, "while", TOKEN_KW_WHILE);

    // Operators with more characters should have higher priority
    TRY_MATCH_STATIC(ls, "&&", TOKEN_OP_AND);
    TRY_MATCH_STATIC(ls, "==", TOKEN_OP_EQ);
    TRY_MATCH_STATIC(ls, ">=", TOKEN_OP_GTE);
    TRY_MATCH_STATIC(ls, "<=", TOKEN_OP_LTE);
    TRY_MATCH_STATIC(ls, "!=", TOKEN_OP_NEQ);
    TRY_MATCH_STATIC(ls, "||", TOKEN_OP_OR);
    TRY_MATCH_STATIC(ls, "+", TOKEN_OP_ADD);
    TRY_MATCH_STATIC(ls, "=", TOKEN_OP_ASSIGN);
    TRY_MATCH_STATIC(ls, ":", TOKEN_OP_COLON);
    TRY_MATCH_STATIC(ls, ",", TOKEN_OP_COMMA);
    TRY_MATCH_STATIC(ls, "/", TOKEN_OP_DIV);
    TRY_MATCH_STATIC(ls, ".", TOKEN_OP_DOT);
    TRY_MATCH_STATIC(ls, ">", TOKEN_OP_GT);
    TRY_MATCH_STATIC(ls, "{", TOKEN_OP_LBRACE);
    TRY_MATCH_STATIC(ls, "[", TOKEN_OP_LBRACK);
    TRY_MATCH_STATIC(ls, "(", TOKEN_OP_LPAREN);
    TRY_MATCH_STATIC(ls, "<", TOKEN_OP_LT);
    TRY_MATCH_STATIC(ls, "%", TOKEN_OP_MOD);
    TRY_MATCH_STATIC(ls, "*", TOKEN_OP_MUL);
    TRY_MATCH_STATIC(ls, "!", TOKEN_OP_NOT);
    TRY_MATCH_STATIC(ls, "}", TOKEN_OP_RBRACE);
    TRY_MATCH_STATIC(ls, "]", TOKEN_OP_RBRACK);
    TRY_MATCH_STATIC(ls, ")", TOKEN_OP_RPAREN);
    TRY_MATCH_STATIC(ls, "-", TOKEN_OP_SUB);

    TRY_MATCH_NAME(ls);
    TRY_MATCH_NUMBER(ls);

    TRY_MATCH_EOF(ls);

    return (struct Token) {
        .type = TOKEN_UNKNOWN,
        .pos = ls->cursor,
        .len = 0,
        .line = ls->curr_line,
        .column = ls->curr_column,
    };
}
