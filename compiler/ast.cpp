#include <sstream>
#include <variant>

#include "ast.hpp"
#include "util.hpp"

namespace fla::compiler::ast
{
    Arena::Arena() : _arena()
    {
    }

    NodeIndex Arena::insert(const Node n)
    {
        _arena.push_back(n);
        return _arena.size() - 1;
    }

    const Node &Arena::get(const NodeIndex ni) const
    {
        return _arena.at(ni);
    }

    const std::string Arena::node_string(const NodeIndex ni) const
    {
        return std::visit(
            util::overloaded {
                [](const Literal &n) -> std::string {
                    if (const auto *int_val { std::get_if<int>(&n.value) }) {
                        return std::format("literal `(int) {}`", *int_val);
                    } else if (const auto *bool_val { std::get_if<bool>(&n.value) }) {
                        return std::format("literal `(bool) {}`", *bool_val);
                    } else {
                        return "literal null";
                    }
                },
                [](const Name &n) -> std::string { return n.name; },
                [](const Add &) -> std::string { return "addition"; },
                [](const And &) -> std::string { return "logical and"; },
                [](const Assign &) -> std::string { return "assignment"; },
                [](const ClassDeclaration &) -> std::string { return "class declaration"; },
                [](const ClassDefinition &) -> std::string { return "class definition"; },
                [](const ConstantDeclaration &) -> std::string { return "constant declaration"; },
                [](const Div &) -> std::string { return "division"; },
                [](const Eq &) -> std::string { return "equality checking"; },
                [](const ElseBranch &) -> std::string { return "else branch"; },
                [](const ExpressionGroup &) -> std::string { return "expression group"; },
                [](const FunctionDeclaration &) -> std::string { return "function declaration"; },
                [](const FunctionDefinition &) -> std::string { return "function definition"; },
                [](const Gt &) -> std::string { return "greater-than comparison"; },
                [](const Gte &) -> std::string { return "greater-than or equal comparison"; },
                [](const IfExpression &) -> std::string { return "if expression"; },
                [](const InterfaceDefinition &) -> std::string { return "interface definition"; },
                [](const Lt &) -> std::string { return "less-than comparison"; },
                [](const Lte &) -> std::string { return "less-than or equal comparison"; },
                [](const Mod &) -> std::string { return "modulo"; },
                [](const Mul &) -> std::string { return "multiplication"; },
                [](const NamespaceDeclaration &) -> std::string { return "namespace declaration"; },
                [](const Neg &) -> std::string { return "negation"; },
                [](const Neq &) -> std::string { return "inequality checking"; },
                [](const Not &) -> std::string { return "logical not"; },
                [](const Or &) -> std::string { return "logical or"; },
                [](const PublicScope &) -> std::string { return "public scope"; },
                [](const Root &) -> std::string { return "root"; },
                [](const Sub &) -> std::string { return "subtraction"; },
                [](const UseDeclaration &) -> std::string { return "use declaration"; },
                [](const VariableDeclaration &) -> std::string { return "variable declaration"; },
                [](const WhileLoop &) -> std::string { return "while loop"; },

                [this](const TypeNotation &n) -> std::string {
                    return std::format("type notation `{}`", node_string(n.tn));
                },
                [this](const ArrayTypeNotation &tn) -> std::string {
                    return std::format("[]{}", node_string(tn.element_tn));
                },
                [this](const FunctionTypeNotation &tn) -> std::string {
                    std::string return_tn { "" };
                    if (tn.return_tn) {
                        return_tn += " " + node_string(*tn.return_tn);
                    }
                    return std::format("fun(){}", return_tn);
                },

                [](const auto &) -> std::string { std::unreachable(); },
            },
            _arena.at(ni));
    }

    const Metadata &Arena::node_metadata(const NodeIndex ni) const
    {
        return std::visit<const Metadata &>(
            [](const auto &n) -> const Metadata & {
                if constexpr (requires { n->meta; }) {
                    return n->meta;
                } else {
                    return n.meta;
                }
            },
            _arena.at(ni));
    }

    const std::string NamespaceDeclaration::string(const Arena &arena) const
    {
        std::ostringstream s;

        for (std::size_t i = 0; i < name_segments.size(); i++) {
            const auto *name { std::get_if<Name>(&arena.get(name_segments.at(i))) };
            s << name->name;
            if (i < name_segments.size() - 1) {
                s << ".";
            }
        }

        return s.str();
    }
} // namespace fla::compiler::ast
