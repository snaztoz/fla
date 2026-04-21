#ifndef ORCHID_PARSER_H
#define ORCHID_PARSER_H

#include <string_view>

#include "ast.hpp"
#include "lexer.hpp"

namespace orchid::compiler
{
    class Parser
    {
    public:
        Parser(std::string_view src);
        Ast parse();

    private:
        Lexer lexer;
    };
} // namespace orchid::compiler

#endif
