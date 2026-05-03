#include <cstdio>
#include <print>

#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace orchid::compiler
{
    int compile(std::string_view src)
    {
        Parser parser { src };

        if (auto res = parser.parse(); !res)
        {
            std::println(stderr, "error: failed to parse ({})", res.error());
            return 1;
        }

        for (const auto &n : parser.ast.arena)
        {
            std::println("{}", node_type_string(n.type));
        }

        return 0;
    }
} // namespace orchid::compiler
