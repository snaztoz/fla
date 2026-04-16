#include <stdio.h> // printf

#include "compiler.h"
#include "lexer.h"    // next_token, struct LexerState
#include "token.h"    // TOKEN_*

int orchid_compile(const char *src)
{
    struct LexerState ls = {
        .src = src,
        .cursor = 0,
        .curr_line = 1,
        .curr_column = 1,
    };

    while (1) {
        struct Token t = next_token(&ls);
        printf("(\"%.*s\", %ld, %ld)\n", t.len, ls.src + t.pos, t.line, t.column);
        if (t.type == TOKEN_EOF || t.type == TOKEN_UNKNOWN) {
            break;
        }
    }

    return 0;
}
