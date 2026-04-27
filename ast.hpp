#ifndef ORCHID_AST_H
#define ORCHID_AST_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace orchid::compiler
{
    enum class NodeType
    {
        Root,
        NamespaceDeclaration,
        Name,
    };

    std::string_view node_type_sv(NodeType &nt);

    struct Node
    {
        NodeType type;
        std::variant<std::nullptr_t, std::string_view, int> value;
        std::optional<std::vector<std::size_t>> children_idx;
    };

    struct Ast
    {
        std::vector<Node> arena;
        std::size_t root_idx;
    };
} // namespace orchid::compiler

#endif
