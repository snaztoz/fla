#include <string_view>

#include "parser.hpp"

namespace orchid::compiler
{
    Parser::Parser(std::string_view src) : lexer(src)
    {
    }

    void Parser::parse()
    {
    }
} // namespace orchid::compiler
