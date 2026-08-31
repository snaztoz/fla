#include <expected>
#include <utility>
#include <variant>

#include "ast.hpp"
#include "context.hpp"
#include "error.hpp"
#include "type_check.hpp"

namespace fla::compiler::type_check
{
    bool is_missing_namespace(const ast::Arena &, const std::vector<ast::NodeIndex> &);
    const std::string read_namespace_string(const ast::Arena &, const ast::NodeIndex node_ni);

    const Result read_declarations(const ast::Arena &arena, const ast::NodeIndex root_ni)
    {
        Namespace ns;

        const auto *r { std::get_if<ast::Root>(&arena.get(root_ni)) };
        if (!r) {
            return std::unexpected(Error { 0, 0, 0, 0, "failed to match root" });
        }

        if (is_missing_namespace(arena, r->body)) {
            return std::unexpected(Error { 0, 0, 0, 0, "missing namespace declaration" });
        }

        ns.name = read_namespace_string(arena, r->body.at(0));

        return ns;
    }

    bool is_missing_namespace(const ast::Arena &arena, const std::vector<ast::NodeIndex> &body)
    {
        return body.size() == 0 ||
               !std::holds_alternative<ast::NamespaceDeclaration>(arena.get(body.at(0)));
    }

    const std::string read_namespace_string(const ast::Arena &arena, const ast::NodeIndex ns_node)
    {
        const auto *ns { std::get_if<ast::NamespaceDeclaration>(&arena.get(ns_node)) };
        if (!ns) {
            std::unreachable();
        }

        return ns->string(arena);
    }
}; // namespace fla::compiler::type_check
