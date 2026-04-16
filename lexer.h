#ifndef ORCHID_LEXER_H
#define ORCHID_LEXER_H

#include <stddef.h> // size_t

#include "token.h"  // struct Token

struct LexerState {
    const char *src;
    size_t     cursor;
    size_t     curr_line;
    size_t     curr_column;
};

struct Token next_token(struct LexerState *ls);

#endif
