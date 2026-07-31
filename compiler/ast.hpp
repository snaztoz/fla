#ifndef FLA_COMPILER_AST_H
#define FLA_COMPILER_AST_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace fla::compiler
{
    struct Metadata {
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t col;
    };

    struct Literal {
        std::variant<std::nullptr_t, int, bool> value;
        Metadata meta;
    };

    struct Name {
        std::string name;
        Metadata meta;
    };

    struct ArrayTypeNotation;
    struct FunctionTypeNotation;

    using TypeNotation = std::variant<Name, std::unique_ptr<ArrayTypeNotation>,
                                      std::unique_ptr<FunctionTypeNotation>>;

    struct ArrayTypeNotation {
        TypeNotation element_tn;
        Metadata meta;
    };

    struct FunctionTypeNotation {
        std::vector<TypeNotation> parameter_tns;
        std::optional<TypeNotation> return_tn;
        Metadata meta;
    };

    struct TypeNotationNode {
        TypeNotation tn;
        Metadata meta;
    };

    struct Add;
    struct And;
    struct Assign;
    struct ClassDefinition;
    struct ConstantDeclaration;
    struct Div;
    struct ElseBranch;
    struct Eq;
    struct ExpressionGroup;
    struct FunctionDefinition;
    struct Gt;
    struct Gte;
    struct IfExpression;
    struct Lt;
    struct Lte;
    struct Mod;
    struct Mul;
    struct NamespaceDeclaration;
    struct Neg;
    struct Neq;
    struct Not;
    struct Or;
    struct Root;
    struct Sub;
    struct UseDeclaration;
    struct VariableDeclaration;
    struct WhileLoop;

    using Node = std::variant<
        Literal, Name, TypeNotationNode, std::unique_ptr<Add>, std::unique_ptr<And>,
        std::unique_ptr<Assign>, std::unique_ptr<ClassDefinition>,
        std::unique_ptr<ConstantDeclaration>, std::unique_ptr<Div>, std::unique_ptr<ElseBranch>,
        std::unique_ptr<Eq>, std::unique_ptr<ExpressionGroup>, std::unique_ptr<FunctionDefinition>,
        std::unique_ptr<Gt>, std::unique_ptr<Gte>, std::unique_ptr<IfExpression>,
        std::unique_ptr<Lt>, std::unique_ptr<Lte>, std::unique_ptr<Mod>, std::unique_ptr<Mul>,
        std::unique_ptr<NamespaceDeclaration>, std::unique_ptr<Neg>, std::unique_ptr<Neq>,
        std::unique_ptr<Not>, std::unique_ptr<Or>, std::unique_ptr<Root>, std::unique_ptr<Sub>,
        std::unique_ptr<UseDeclaration>, std::unique_ptr<VariableDeclaration>,
        std::unique_ptr<WhileLoop>>;

    struct Add {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct And {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Assign {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct ClassDefinition {
        Name name;
        std::vector<Node> body;
        Metadata meta;
    };

    struct ConstantDeclaration {
        Name name;
        std::optional<Node> type_notation;
        Node expression;
        Metadata meta;
    };

    struct Div {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Eq {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct ElseBranch {
        std::vector<Node> body;
        Metadata meta;
    };

    struct ExpressionGroup {
        Node expression;
        Metadata meta;
    };

    struct FunctionDefinition {
        Name name;
        std::vector<std::pair<Name, Node>> parameters;
        std::optional<Node> return_type_notation;
        std::vector<Node> body;
        Metadata meta;
    };

    struct Gt {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Gte {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct IfExpression {
        Node cond;
        std::vector<Node> body;
        std::optional<Node> else_statement;
        Metadata meta;
    };

    struct Lt {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Lte {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Mod {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Mul {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct NamespaceDeclaration {
        std::vector<Name> name_segments;
        Metadata meta;
    };

    struct Neg {
        Node expression;
        Metadata meta;
    };

    struct Neq {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Not {
        Node expression;
        Metadata meta;
    };

    struct Or {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct Root {
        std::vector<Node> body;
        Metadata meta;
    };

    struct Sub {
        Node lhs;
        Node rhs;
        Metadata meta;
    };

    struct UseDeclaration {
        std::vector<Name> name_segments;
        Metadata meta;
    };

    struct VariableDeclaration {
        Name name;
        std::optional<Node> type_notation;
        std::optional<Node> expression;
        Metadata meta;
    };

    struct WhileLoop {
        Node cond;
        std::vector<Node> body;
        Metadata meta;
    };

    template <class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };

    std::string get_node_repr(const Node &node);
    const Metadata &get_node_metadata(const Node &node);

    std::string get_type_notation_node_repr(const TypeNotationNode &tn);
    std::string get_type_notation_repr(const TypeNotation &tn);
    const Metadata &get_type_notation_metadata(const TypeNotation &tn);
} // namespace fla::compiler

#endif
