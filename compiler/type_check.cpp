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
    const std::string read_namespace_string(const ast::Arena &, const ast::Root &);

    TypeMapping read_public_deferred_types(const ast::Arena &,
                                           const std::vector<ast::NodeIndex> &body);
    TypeMapping read_deferred_types(const ast::Arena &, const std::vector<ast::NodeIndex> &body,
                                    const bool is_public);

    TypeMapping read_public_types(const ast::Arena &, const std::vector<ast::NodeIndex> &body);
    TypeMapping read_types(const ast::Arena &, const std::vector<ast::NodeIndex> &body,
                           const bool is_public);

    const std::expected<void, Error> resolve_deferred_types(Namespace &, const ast::Arena &);
    constexpr bool should_promote_visibility(const Type &concrete_type, const Type &deferred_type);

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

        ns.name = read_namespace_string(arena, *r);
        ns.deferred_types.merge(read_deferred_types(arena, r->body, false));
        ns.deferred_types.merge(read_public_deferred_types(arena, r->body));
        ns.types.merge(read_types(arena, r->body, false));
        ns.types.merge(read_public_types(arena, r->body));

        if (const auto ok { resolve_deferred_types(ns, arena) }; !ok) {
            return std::unexpected(ok.error());
        }

        return ns;
    }

    bool is_missing_namespace(const ast::Arena &arena, const std::vector<ast::NodeIndex> &body)
    {
        return body.size() == 0 ||
               !std::holds_alternative<ast::NamespaceDeclaration>(arena.get(body.at(0)));
    }

    const std::string read_namespace_string(const ast::Arena &arena, const ast::Root &root)
    {
        const auto *ns { std::get_if<ast::NamespaceDeclaration>(&arena.get(root.body.at(0))) };
        if (!ns) {
            std::unreachable();
        }

        return ns->string(arena);
    }

    TypeMapping read_public_deferred_types(const ast::Arena &arena,
                                           const std::vector<ast::NodeIndex> &body)
    {
        TypeMapping types;

        for (const auto &n : body) {
            const auto *public_node { std::get_if<ast::PublicScope>(&arena.get(n)) };
            if (!public_node) {
                continue;
            }

            auto scope_types { read_deferred_types(arena, public_node->body, true) };

            types.merge(scope_types);
        }

        return types;
    }

    TypeMapping read_deferred_types(const ast::Arena &arena,
                                    const std::vector<ast::NodeIndex> &body, const bool is_public)
    {
        TypeMapping types;

        for (const auto &n : body) {
            if (const auto *t { std::get_if<ast::ClassDeclaration>(&arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&arena.get(t->name)) };
                types.insert({
                    name->name,
                    { name->name, TypeVariant::ClassDeclaration, n, is_public },
                });
                continue;
            }
        }

        return types;
    }

    TypeMapping read_public_types(const ast::Arena &arena, const std::vector<ast::NodeIndex> &body)
    {
        TypeMapping types;

        for (const auto &n : body) {
            const auto *public_node { std::get_if<ast::PublicScope>(&arena.get(n)) };
            if (!public_node) {
                continue;
            }

            auto scope_types { read_types(arena, public_node->body, true) };

            types.merge(scope_types);
        }

        return types;
    }

    TypeMapping read_types(const ast::Arena &arena, const std::vector<ast::NodeIndex> &body,
                           const bool is_public)
    {
        TypeMapping types;

        for (const auto &n : body) {
            if (const auto *t { std::get_if<ast::ClassDefinition>(&arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&arena.get(t->name)) };
                types.insert({
                    name->name,
                    { name->name, TypeVariant::Class, n, is_public },
                });
                continue;
            }

            if (const auto *t { std::get_if<ast::InterfaceDefinition>(&arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&arena.get(t->name)) };
                types.insert({
                    name->name,
                    { name->name, TypeVariant::Interface, n, is_public },
                });
                continue;
            }
        }

        return types;
    }

    const std::expected<void, Error> resolve_deferred_types(Namespace &ns, const ast::Arena &arena)
    {
        for (const auto &[name, t] : ns.deferred_types) {
            if (!ns.types.contains(name)) {
                const auto ni { ns.deferred_types.at(name).ni };
                const auto meta { arena.get_node_metadata(ni) };

                return std::unexpected(Error {
                    meta.pos,
                    meta.len,
                    meta.line,
                    meta.col,
                    "missing class definition",
                });
            }

            if (should_promote_visibility(ns.types.at(name), t)) {
                ns.types.at(name).is_public = true;
            }
        }

        return {};
    }

    constexpr bool should_promote_visibility(const Type &concrete_type, const Type &deferred_type)
    {
        return deferred_type.is_public && !concrete_type.is_public;
    }
}; // namespace fla::compiler::type_check
