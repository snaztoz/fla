#ifndef ORCHID_TOKEN_H
#define ORCHID_TOKEN_H

#include <stddef.h> // size_t

struct Token {
    const int    type;
    const size_t pos;
    const int    len;
    const size_t line;
    const size_t column;
};

#define TOKEN_UNKNOWN     0
#define TOKEN_EOF         1
#define TOKEN_NAME        2

#define TOKEN_KW_CLASS    11
#define TOKEN_KW_ELSE     12
#define TOKEN_KW_FUNCTION 13
#define TOKEN_KW_IF       14
#define TOKEN_KW_IMPORT   15
#define TOKEN_KW_PACKAGE  16
#define TOKEN_KW_VAR      17
#define TOKEN_KW_WHILE    18

#define TOKEN_OP_ADD      31
#define TOKEN_OP_AND      32
#define TOKEN_OP_ASSIGN   33
#define TOKEN_OP_COLON    34
#define TOKEN_OP_COMMA    35
#define TOKEN_OP_DIV      36
#define TOKEN_OP_DOT      37
#define TOKEN_OP_EQ       38
#define TOKEN_OP_GT       39
#define TOKEN_OP_GTE      40
#define TOKEN_OP_LBRACE   41
#define TOKEN_OP_LBRACK   42
#define TOKEN_OP_LPAREN   43
#define TOKEN_OP_LT       44
#define TOKEN_OP_LTE      45
#define TOKEN_OP_MOD      46
#define TOKEN_OP_MUL      47
#define TOKEN_OP_NEQ      48
#define TOKEN_OP_NOT      49
#define TOKEN_OP_OR       50
#define TOKEN_OP_RBRACE   51
#define TOKEN_OP_RBRACK   52
#define TOKEN_OP_RPAREN   53
#define TOKEN_OP_SUB      54

#endif
