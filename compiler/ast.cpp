#include <format>
#include <memory>
#include <variant>

#include "ast.hpp"

namespace fla::compiler
{
    std::string get_node_repr(const Node &node)
    {
        return std::visit(
            overloaded {
                [](const Literal &n) -> std::string {
                    if (const auto *int_val { std::get_if<int>(&n.value) }) {
                        return std::format("literal `(int) {}`", *int_val);
                    } else if (const auto *bool_val { std::get_if<bool>(&n.value) }) {
                        return std::format("literal `(bool) {}`", *bool_val);
                    } else {
                        return "literal null";
                    }
                },
                [](const Name &n) -> std::string { return std::format("name `{}`", n.name); },
                [](const TypeNotationNode &n) -> std::string {
                    return std::format("type notation `{}`", get_type_notation_node_repr(n));
                },
                [](const std::unique_ptr<Add> &) -> std::string { return "addition"; },
                [](const std::unique_ptr<And> &) -> std::string { return "logical and"; },
                [](const std::unique_ptr<Assign> &) -> std::string { return "assignment"; },
                [](const std::unique_ptr<ClassDeclaration> &) -> std::string {
                    return "class declaration";
                },
                [](const std::unique_ptr<ClassDefinition> &) -> std::string {
                    return "class definition";
                },
                [](const std::unique_ptr<ConstantDeclaration> &) -> std::string {
                    return "constant declaration";
                },
                [](const std::unique_ptr<Div> &) -> std::string { return "division"; },
                [](const std::unique_ptr<Eq> &) -> std::string { return "equality checking"; },
                [](const std::unique_ptr<ElseBranch> &) -> std::string { return "else branch"; },
                [](const std::unique_ptr<ExpressionGroup> &) -> std::string {
                    return "expression group";
                },
                [](const std::unique_ptr<FunctionDeclaration> &) -> std::string {
                    return "function declaration";
                },
                [](const std::unique_ptr<FunctionDefinition> &) -> std::string {
                    return "function definition";
                },
                [](const std::unique_ptr<Gt> &) -> std::string {
                    return "greater-than comparison";
                },
                [](const std::unique_ptr<Gte> &) -> std::string {
                    return "greater-than or equal comparison";
                },
                [](const std::unique_ptr<IfExpression> &) -> std::string {
                    return "if expression";
                },
                [](const std::unique_ptr<InterfaceDefinition> &) -> std::string {
                    return "interface definition";
                },
                [](const std::unique_ptr<Lt> &) -> std::string { return "less-than comparison"; },
                [](const std::unique_ptr<Lte> &) -> std::string {
                    return "less-than or equal comparison";
                },
                [](const std::unique_ptr<Mod> &) -> std::string { return "modulo"; },
                [](const std::unique_ptr<Mul> &) -> std::string { return "multiplication"; },
                [](const std::unique_ptr<NamespaceDeclaration> &) -> std::string {
                    return "namespace declaration";
                },
                [](const std::unique_ptr<Neg> &) -> std::string { return "negation"; },
                [](const std::unique_ptr<Neq> &) -> std::string { return "inequality checking"; },
                [](const std::unique_ptr<Not> &) -> std::string { return "logical not"; },
                [](const std::unique_ptr<Or> &) -> std::string { return "logical or"; },
                [](const std::unique_ptr<PublicScope> &) -> std::string { return "public scope"; },
                [](const std::unique_ptr<Root> &) -> std::string { return "root"; },
                [](const std::unique_ptr<Sub> &) -> std::string { return "subtraction"; },
                [](const std::unique_ptr<UseDeclaration> &) -> std::string {
                    return "use declaration";
                },
                [](const std::unique_ptr<VariableDeclaration> &) -> std::string {
                    return "variable declaration";
                },
                [](const std::unique_ptr<WhileLoop> &) -> std::string { return "while loop"; },
            },
            node);
    }

    const Metadata &get_node_metadata(const Node &node)
    {
        return std::visit<const Metadata &>(
            [](const auto &n) -> const Metadata & {
                if constexpr (requires { n->meta; }) {
                    return n->meta;
                } else {
                    return n.meta;
                }
            },
            node);
    }

    std::string get_type_notation_node_repr(const TypeNotationNode &tn)
    {
        if (const auto name = std::get_if<Name>(&tn.tn)) {
            return std::format("{}", name->name);
        } else {
            return get_type_notation_repr(tn.tn);
        }
    }

    std::string get_type_notation_repr(const TypeNotation &tn)
    {
        return std::visit(
            overloaded {
                [](const std::unique_ptr<ArrayTypeNotation> &tn) -> std::string {
                    return std::format("[]{}", get_type_notation_repr(tn->element_tn));
                },
                [](const std::unique_ptr<FunctionTypeNotation> &tn) -> std::string {
                    std::string return_tn { "" };
                    if (tn->return_tn) {
                        return_tn += " " + get_type_notation_repr(*tn->return_tn);
                    }
                    return std::format("fun(){}", return_tn);
                },
                [](const Name &tn) -> std::string { return std::format("{}", tn.name); },
            },
            tn);
    }

    const Metadata &get_type_notation_metadata(const TypeNotation &tn)
    {
        return std::visit<const Metadata &>(
            [](const auto &n) -> const Metadata & {
                if constexpr (requires { n->meta; }) {
                    return n->meta;
                } else {
                    return n.meta;
                }
            },
            tn);
    }
} // namespace fla::compiler
