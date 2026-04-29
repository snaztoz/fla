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
        UseDeclaration,
        Name,
    };

    constexpr std::string_view node_type_string(NodeType &nt) noexcept
    {
        switch (nt)
        {
        case NodeType::Root:
            return "root";
        case NodeType::NamespaceDeclaration:
            return "namespace declaration";
        case NodeType::UseDeclaration:
            return "use declaration";
        case NodeType::Name:
            return "name";
        default:
            std::unreachable();
        }
    }

    using NodeVariant = std::variant<std::nullptr_t, std::string_view, int>;
    using NodeChildrenIndexes = std::optional<std::vector<std::size_t>>;

    struct Node
    {
        NodeType type;
        NodeVariant value;
        NodeChildrenIndexes children_idx;
    };

    struct Ast
    {
        std::vector<Node> arena;
        std::size_t root_idx;
    };
} // namespace orchid::compiler

#endif
