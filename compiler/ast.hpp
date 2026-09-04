#ifndef FLA_COMPILER_AST_H
#define FLA_COMPILER_AST_H

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace fla::compiler::ast
{
    using NodeIndex = std::size_t;
    using NodeList = std::vector<NodeIndex>;

    struct Metadata {
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t col;
    };

    struct Add;
    struct And;
    struct ArrayTypeNotation;
    struct Assign;
    struct ClassDeclaration;
    struct ClassDefinition;
    struct ConstantDeclaration;
    struct Div;
    struct ElseBranch;
    struct Eq;
    struct ExpressionGroup;
    struct FunctionDeclaration;
    struct FunctionDefinition;
    struct FunctionTypeNotation;
    struct InterfaceDefinition;
    struct Gt;
    struct Gte;
    struct IfExpression;
    struct Literal;
    struct Lt;
    struct Lte;
    struct Mod;
    struct Mul;
    struct Name;
    struct NamespaceDeclaration;
    struct Neg;
    struct Neq;
    struct Not;
    struct Or;
    struct PublicScope;
    struct Root;
    struct Sub;
    struct TypeNotation;
    struct UseDeclaration;
    struct VariableDeclaration;
    struct WhileLoop;

    using Node =
        std::variant<Add, And, ArrayTypeNotation, Assign, ClassDeclaration, ClassDefinition,
                     ConstantDeclaration, Div, ElseBranch, Eq, ExpressionGroup, FunctionDeclaration,
                     FunctionDefinition, FunctionTypeNotation, InterfaceDefinition, Gt, Gte,
                     IfExpression, Literal, Lt, Lte, Mod, Mul, Name, NamespaceDeclaration, Neg, Neq,
                     Not, Or, PublicScope, Root, Sub, TypeNotation, UseDeclaration,
                     VariableDeclaration, WhileLoop>;

    class Arena
    {
        std::vector<Node> _arena;

    public:
        explicit Arena();
        NodeIndex insert(const Node);
        const Node &get(const NodeIndex) const;
        const std::string node_string(const NodeIndex) const;
        const Metadata &node_metadata(const NodeIndex) const;
    };

    struct Literal {
        const std::variant<std::nullptr_t, int, bool> value;
        const Metadata meta;
    };

    struct Name {
        const std::string name;
        const Metadata meta;
    };

    struct TypeNotation {
        const NodeIndex tn;
        const Metadata meta;
    };

    struct ArrayTypeNotation {
        const NodeIndex element_tn;
        const Metadata meta;
    };

    struct FunctionTypeNotation {
        const ast::NodeList parameter_tns;
        const std::optional<NodeIndex> return_tn;
        const Metadata meta;
    };

    struct Add {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct And {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Assign {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct ClassDeclaration {
        const NodeIndex name;
        const Metadata meta;
    };

    struct ClassDefinition {
        const NodeIndex name;
        const ast::NodeList body;
        const Metadata meta;
    };

    struct ConstantDeclaration {
        const NodeIndex name;
        const std::optional<NodeIndex> type_notation;
        const NodeIndex expression;
        const Metadata meta;
    };

    struct Div {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Eq {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct ElseBranch {
        const ast::NodeList body;
        const Metadata meta;
    };

    struct ExpressionGroup {
        const NodeIndex expression;
        const Metadata meta;
    };

    struct FunctionDeclaration {
        const NodeIndex name;
        const std::vector<std::pair<NodeIndex, NodeIndex>> parameters;
        const NodeIndex return_tn;
        const Metadata meta;
    };

    struct FunctionDefinition {
        const NodeIndex name;
        const std::vector<std::pair<NodeIndex, NodeIndex>> parameters;
        const std::optional<NodeIndex> return_type_notation;
        const ast::NodeList body;
        const Metadata meta;
    };

    struct Gt {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Gte {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct IfExpression {
        const NodeIndex cond;
        const ast::NodeList body;
        const std::optional<NodeIndex> else_statement;
        const Metadata meta;
    };

    struct InterfaceDefinition {
        const NodeIndex name;
        const ast::NodeList body;
        const Metadata meta;
    };

    struct Lt {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Lte {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Mod {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Mul {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct NamespaceDeclaration {
        const ast::NodeList name_segments;
        const Metadata meta;

        const std::string string(const Arena &) const;
    };

    struct Neg {
        const NodeIndex expression;
        const Metadata meta;
    };

    struct Neq {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct Not {
        const NodeIndex expression;
        const Metadata meta;
    };

    struct Or {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct PublicScope {
        const ast::NodeList body;
        const Metadata meta;
    };

    struct Root {
        const ast::NodeList body;
        const Metadata meta;
    };

    struct Sub {
        const NodeIndex lhs;
        const NodeIndex rhs;
        const Metadata meta;
    };

    struct UseDeclaration {
        const ast::NodeList name_segments;
        const Metadata meta;
    };

    struct VariableDeclaration {
        const NodeIndex name;
        const std::optional<NodeIndex> type_notation;
        const std::optional<NodeIndex> expression;
        const Metadata meta;
    };

    struct WhileLoop {
        const NodeIndex cond;
        const ast::NodeList body;
        const Metadata meta;
    };
} // namespace fla::compiler::ast

#endif
