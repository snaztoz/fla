#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <print>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "compiler.hpp"
#include "error.hpp"
#include "fla/compiler.h"
#include "parser.hpp"
#include "util.hpp"

extern "C" {
int fla_compile(const char *entrypoint, struct FlaCompilerError *err)
{
    try {
        const auto result { fla::compiler::compile(entrypoint) };
        if (!result) {
            throw result.error();
        }
    } catch (const fla::compiler::Error &e) {
        err->pos = e.pos;
        err->len = e.len;
        err->line = e.line;
        err->col = e.col;

        const auto msg_len { e.msg.length() + 1 };

        err->msg = static_cast<char *>(std::malloc(msg_len));
        if (err->msg == nullptr) {
            return 2;
        }

        std::memcpy(err->msg, e.msg.c_str(), msg_len);

        return 1;
    } catch (const std::exception &e) {
        const auto msg_len { std::strlen(e.what()) + 1 };

        err->msg = static_cast<char *>(std::malloc(msg_len));
        if (err->msg == nullptr) {
            return 2;
        }

        std::memcpy(err->msg, e.what(), msg_len);

        return 3;
    }

    return 0;
}

int fla_free_compiler_error(struct FlaCompilerError *err)
{
    if (err && err->msg) {
        std::free(err->msg);
    }
    return 0;
}
}

namespace fla::compiler
{
    const std::filesystem::path STD_DIR { "std" };
    const auto KERNEL_DIR { "kernel" };
    const auto KERNEL_IO_FILE { "io.fla" };
    const auto KERNEL_TYPE_FILE { "type.fla" };

    enum EntityVariant {
        Class,
        Interface,
    };

    struct Entity {
        std::string name;
        EntityVariant variant;
    };

    struct Namespace {
        std::string name;
        std::unordered_map<std::string, Entity> public_entities;
    };

    struct CompilerContext {
        std::unordered_map<std::string, Namespace> namespaces;
    };

    void print_node(const Node &node, const int level);

    std::expected<void, Error> compile(const std::filesystem::path entrypoint)
    {
        CompilerContext ctx {};

        std::vector<std::filesystem::path> files {
            STD_DIR / KERNEL_DIR / KERNEL_TYPE_FILE,
            STD_DIR / KERNEL_DIR / KERNEL_IO_FILE,
            entrypoint,
        };

        for (const auto &file : files) {
            const auto content { util::read_file(file) };

            const auto root { parse(*content) };
            if (!root) {
                return std::unexpected(root.error());
            }

            std::println("#[{}]\n", file.string());
            print_node(*root, 0);
            std::println();
        }

        return {};
    }

    void print_node(const Node &node, const int level)
    {
        const std::string indentation(level * 2, ' ');

        std::print("{}{}", indentation, get_node_repr(node));

        const Metadata meta { get_node_metadata(node) };
        std::print(" ({}:{}:{})\n", meta.line, meta.col, meta.len);

        std::visit(
            overloaded {
                [](const Literal &) {},
                [](const Name &) {},
                [](const TypeNotationNode &) {},
                [level](const std::unique_ptr<Add> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<And> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Assign> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<ClassDefinition> &n) {
                    print_node(n->name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');
                    if (!n->body.empty()) {
                        std::println("{}{{body}}", child_indentation);
                        for (const auto &statement : n->body) {
                            print_node(statement, level + 2);
                        }
                    }
                },
                [level](const std::unique_ptr<ClassForwardDeclaration> &n) {
                    print_node(n->name, level + 1);
                },
                [level](const std::unique_ptr<ConstantDeclaration> &n) {
                    print_node(n->name, level + 1);
                    if (n->type_notation) {
                        print_node(*n->type_notation, level + 1);
                    }
                    print_node(n->expression, level + 1);
                },
                [level](const std::unique_ptr<Div> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Eq> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<ElseBranch> &n) {
                    for (const auto &statement : n->body) {
                        print_node(statement, level + 1);
                    }
                },
                [level](const std::unique_ptr<ExpressionGroup> &n) {
                    print_node(n->expression, level + 1);
                },
                [level](const std::unique_ptr<FunctionDefinition> &n) {
                    print_node(n->name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');

                    for (const auto &param : n->parameters) {
                        std::println("{}{{parameter}}", child_indentation);
                        print_node(param.first, level + 2);
                        print_node(param.second, level + 2);
                    }

                    if (n->return_type_notation) {
                        std::println("{}{{return}}", child_indentation);
                        print_node(*n->return_type_notation, level + 2);
                    }

                    if (!n->body.empty()) {
                        std::println("{}{{body}}", child_indentation);
                        for (const auto &statement : n->body) {
                            print_node(statement, level + 2);
                        }
                    }
                },
                [level](const std::unique_ptr<FunctionForwardDeclaration> &n) {
                    print_node(n->name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');

                    for (const auto &param : n->parameters) {
                        std::println("{}{{parameter}}", child_indentation);
                        print_node(param.first, level + 2);
                        print_node(param.second, level + 2);
                    }

                    std::println("{}{{return}}", child_indentation);
                    print_node(n->return_tn, level + 2);
                },
                [level](const std::unique_ptr<Gt> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Gte> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<IfExpression> &n) {
                    print_node(n->cond, level + 1);
                    for (const auto &statement : n->body) {
                        print_node(statement, level + 1);
                    }
                    if (n->else_statement) {
                        print_node(*n->else_statement, level + 1);
                    }
                },
                [level](const std::unique_ptr<InterfaceDefinition> &n) {
                    print_node(n->name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');
                    if (!n->body.empty()) {
                        std::println("{}{{body}}", child_indentation);
                        for (const auto &statement : n->body) {
                            print_node(statement, level + 2);
                        }
                    }
                },
                [level](const std::unique_ptr<Lt> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Lte> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Mod> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Mul> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<NamespaceDeclaration> &n) {
                    for (const auto &segment : n->name_segments) {
                        print_node(segment, level + 1);
                    }
                },
                [level](const std::unique_ptr<Neg> &n) { print_node(n->expression, level + 1); },
                [level](const std::unique_ptr<Neq> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<Not> &n) { print_node(n->expression, level + 1); },
                [level](const std::unique_ptr<Or> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<PublicScope> &n) {
                    for (const auto &statement : n->body) {
                        print_node(statement, level + 1);
                    }
                },
                [level](const std::unique_ptr<Root> &n) {
                    for (const auto &statement : n->body) {
                        print_node(statement, level + 1);
                    }
                },
                [level](const std::unique_ptr<Sub> &n) {
                    print_node(n->lhs, level + 1);
                    print_node(n->rhs, level + 1);
                },
                [level](const std::unique_ptr<UseDeclaration> &n) {
                    for (const auto &segment : n->name_segments) {
                        print_node(segment, level + 1);
                    }
                },
                [level](const std::unique_ptr<VariableDeclaration> &n) {
                    print_node(n->name, level + 1);
                    if (n->type_notation) {
                        print_node(*n->type_notation, level + 1);
                    }
                    if (n->expression) {
                        print_node(*n->expression, level + 1);
                    }
                },
                [level](const std::unique_ptr<WhileLoop> &n) {
                    print_node(n->cond, level + 1);
                    for (const auto &statement : n->body) {
                        print_node(statement, level + 1);
                    }
                },
            },
            node);
    }
} // namespace fla::compiler
