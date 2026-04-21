#ifndef ORCHID_AST_H
#define ORCHID_AST_H

#include <cstddef>
#include <string_view>
#include <variant>
#include <vector>

namespace orchid::compiler
{
    enum class NodeType
    {
        Root,
    };

    struct Node
    {
        const NodeType type;
        const std::variant<std::nullptr_t, std::string_view, int> value;
        const std::vector<std::size_t> children_idx;
    };

    struct Ast
    {
        const std::vector<Node> arena;
        const std::size_t root_idx;
    };
} // namespace orchid::compiler

#endif
