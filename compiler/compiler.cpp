#include <cstdio>
#include <print>
#include <string>
#include <variant>

#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace fla::compiler
{
    void print_node(const Node &node, const int level)
    {
        std::string indentation(level * 2, ' ');

        std::print("{}{}", indentation, node_type_string(node.type));

        if (const auto *string_val {
                std::get_if<std::string_view>(&node.value) }) {
            std::print(" -> {}", *string_val);
        } else if (const auto *int_val { std::get_if<int>(&node.value) }) {
            std::print(" -> {}", *int_val);
        }

        std::println();

        for (const auto &child : node.children) {
            print_node(child, level + 1);
        }
    }

    int compile(const std::string_view src)
    {
        Parser parser { src };

        auto root { parser.parse() };
        if (!root) {
            std::println(stderr, "error: failed to parse ({})", root.error());
            return 1;
        }

        print_node(root.value(), 0);

        return 0;
    }
} // namespace fla::compiler
