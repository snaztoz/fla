#include <iostream>

#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace orchid::compiler
{
    int compile(std::string_view src)
    {
        Parser parser { src };
        parser.parse();

        for (const auto& n : parser.ast.arena)
        {
            std::cout << node_type_string(n.type) << "\n";
        }

        return 0;
    }
} // namespace orchid::compiler
