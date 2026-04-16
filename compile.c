#include <ctype.h>  // isspace
#include <stddef.h> // size_t
#include <stdio.h>  // printf
#include <string.h> // strncmp

struct ParserState {
    const char *src;
    size_t     cursor;
    size_t     curr_line;
    size_t     curr_column;
};

struct Token {
    const int    type;
    const char   *text;
    const size_t line;
    const size_t column;
};

#define TOKEN_EOF        0
#define TOKEN_KW_PACKAGE 1
#define TOKEN_OP_DOT     2
#define TOKEN_NAME       3

struct Token next_token(struct ParserState *ps) {
    while (isspace(ps->src[ps->cursor])) {
        if (ps->src[ps->cursor] == '\n') {
            ps->curr_line += 1;
            ps->curr_column = 0; // reset
        }
        ps->curr_column += 1;
        ps->cursor += 1;
    }

    if (strncmp(ps->src + ps->cursor, "package", 7) == 0) {
        struct Token t = {
            .type = TOKEN_KW_PACKAGE,
            .text = "package",
            .line = ps->curr_line,
            .column = ps->curr_column,
        };
        ps->curr_column += 7;
        return t;
    }

    return (struct Token){
        .type = TOKEN_EOF,
        .text = "",
        .line = ps->curr_line,
        .column = ps->curr_column,
    };
}

int orchid_compile(const char *src)
{
    struct ParserState ps = {
        .src = src,
        .cursor = 0,
        .curr_line = 1,
        .curr_column = 1,
    };

    struct Token t = next_token(&ps);

    printf("Type: %d\n", t.type);
    printf("Text: %s\n", t.text);
    printf("Line: %ld\n", t.line);
    printf("Column: %ld\n", t.column);

    return 0;
}
