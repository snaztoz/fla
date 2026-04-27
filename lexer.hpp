#ifndef ORCHID_LEXER_H
#define ORCHID_LEXER_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "token.hpp"

namespace orchid::compiler
{
    class Lexer
    {
    public:
        Lexer(std::string_view src);
        Token next_token();
        Token peek_token();

    private:
        std::string_view src;
        std::size_t cursor;
        std::size_t curr_line;
        std::size_t curr_column;
        std::vector<std::function<std::optional<Token>(void)>> rules;

        std::optional<Token> try_match(std::string_view text,
                                       TokenType type_if_matches);
        std::optional<Token> try_match_name();
        std::optional<Token> try_match_number();
        std::optional<Token> try_match_eof();
        void skip_whitespaces();
        char current();
        bool is_current_valid_name_start();
        bool is_current_valid_name_tail();
    };
} // namespace orchid::compiler

#endif
