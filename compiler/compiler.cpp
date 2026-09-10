#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <print>
#include <string>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "common.hpp"
#include "compiler.hpp"
#include "error.hpp"
#include "fla/compiler.h"
#include "namespace.hpp"
#include "parser.hpp"
#include "type_check.hpp"
#include "util.hpp"

extern "C" {
int fla_compile(const char *entrypoint, const char *std_path, struct FlaCompilerError *err)
{
    try {
        const auto result { fla::compiler::compile(entrypoint, std_path) };
        if (!result) {
            throw result.error();
        }
    } catch (const fla::compiler::error::Error &e) {
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
    using namespace fla::compiler::common;

    const auto KERNEL_DIR { "kernel" };
    const auto KERNEL_IO_FILE { "io.fla" };
    const auto KERNEL_TYPE_FILE { "type.fla" };

    void print_node(const ast::Arena &arena, const ast::NodeIndex node, const int level);

    const VoidResult compile(const std::filesystem::path &entrypoint,
                             const std::filesystem::path &std_path)
    {
        CompilerContext ctx {};

        std::vector<std::filesystem::path> files {
            std_path / KERNEL_DIR / KERNEL_TYPE_FILE,
            std_path / KERNEL_DIR / KERNEL_IO_FILE,
            entrypoint,
        };

        for (const auto &file : files) {
            const auto content { util::read_file(file) };
            if (!content) {
                return std::unexpected(content.error());
            }

            parser::Context parser_ctx { *content };

            const auto root_ni { parse(parser_ctx) };
            if (!root_ni) {
                return std::unexpected(root_ni.error());
            }

            const auto *root { std::get_if<ast::Root>(&parser_ctx.arena.get(*root_ni)) };
            type_check::Context type_check_ctx { std::move(parser_ctx.arena), *root };

            const auto ns { type_check::read_declarations(type_check_ctx) };
            if (!ns) {
                return std::unexpected(ns.error());
            }

            const auto ns_name { ns->name };
            ctx.namespaces.insert({ ns_name, std::move(*ns) });

            // std::println("#[{}]\n", file.string());
            // print_node(parser_ctx.arena, *root, 0);

            // std::println("");
        }

        for (const auto &[_, ns] : ctx.namespaces) {
            std::println("#[{}]", ns.name);

            std::println("    using:");
            for (const auto &[ns_name, t] : ns.external_types) {
                std::println("        {} {}.{}", string(t.variant), ns_name, t.name);
            }

            std::println("    types:");
            for (const auto &[t_name, t] : ns.types) {
                std::print("        ");
                if (t.is_public) {
                    std::print("(public) ");
                }
                std::print("{} {}", string(t.variant), t_name);
                std::println("");
            }

            std::println("");
        }

        return {};
    }

    void print_node(const ast::Arena &arena, const ast::NodeIndex ni, const int level)
    {
        const std::string indentation(level * 2, ' ');

        std::print("{}{}", indentation, arena.node_string(ni));

        const auto meta { arena.node_metadata(ni) };
        std::print(" ({}:{}:{})\n", meta.line, meta.col, meta.len);

        std::visit(
            util::overloaded {
                [](const ast::Literal &) {},
                [](const ast::Name &) {},
                [](const ast::TypeNotation &) {},
                [arena, level](const ast::Add &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::And &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Assign &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::ClassDeclaration &n) {
                    print_node(arena, n.name, level + 1);
                },
                [arena, level](const ast::ClassDefinition &n) {
                    print_node(arena, n.name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');
                    if (!n.body.empty()) {
                        std::println("{}{{body}}", child_indentation);
                        for (const auto &statement : n.body) {
                            print_node(arena, statement, level + 2);
                        }
                    }
                },
                [arena, level](const ast::ConstantDeclaration &n) {
                    print_node(arena, n.name, level + 1);
                    if (n.type_notation) {
                        print_node(arena, *n.type_notation, level + 1);
                    }
                    print_node(arena, n.expression, level + 1);
                },
                [arena, level](const ast::Div &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Eq &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::ElseBranch &n) {
                    for (const auto &statement : n.body) {
                        print_node(arena, statement, level + 1);
                    }
                },
                [arena, level](const ast::ExpressionGroup &n) {
                    print_node(arena, n.expression, level + 1);
                },
                [arena, level](const ast::FunctionDeclaration &n) {
                    print_node(arena, n.name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');

                    for (const auto &param : n.parameters) {
                        std::println("{}{{parameter}}", child_indentation);
                        print_node(arena, param.first, level + 2);
                        print_node(arena, param.second, level + 2);
                    }

                    std::println("{}{{return}}", child_indentation);
                    print_node(arena, n.return_tn, level + 2);
                },
                [arena, level](const ast::FunctionDefinition &n) {
                    print_node(arena, n.name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');

                    for (const auto &param : n.parameters) {
                        std::println("{}{{parameter}}", child_indentation);
                        print_node(arena, param.first, level + 2);
                        print_node(arena, param.second, level + 2);
                    }

                    if (n.return_type_notation) {
                        std::println("{}{{return}}", child_indentation);
                        print_node(arena, *n.return_type_notation, level + 2);
                    }

                    if (!n.body.empty()) {
                        std::println("{}{{body}}", child_indentation);
                        for (const auto &statement : n.body) {
                            print_node(arena, statement, level + 2);
                        }
                    }
                },
                [arena, level](const ast::Gt &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Gte &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::IfExpression &n) {
                    print_node(arena, n.cond, level + 1);
                    for (const auto &statement : n.body) {
                        print_node(arena, statement, level + 1);
                    }
                    if (n.else_statement) {
                        print_node(arena, *n.else_statement, level + 1);
                    }
                },
                [arena, level](const ast::InterfaceDefinition &n) {
                    print_node(arena, n.name, level + 1);

                    const std::string child_indentation((level + 1) * 2, ' ');
                    if (!n.body.empty()) {
                        std::println("{}{{body}}", child_indentation);
                        for (const auto &statement : n.body) {
                            print_node(arena, statement, level + 2);
                        }
                    }
                },
                [arena, level](const ast::Lt &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Lte &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Mod &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Mul &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::NamespaceDeclaration &n) {
                    for (const auto &segment : n.name_segments) {
                        print_node(arena, segment, level + 1);
                    }
                },
                [arena, level](const ast::Neg &n) { print_node(arena, n.expression, level + 1); },
                [arena, level](const ast::Neq &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::Not &n) { print_node(arena, n.expression, level + 1); },
                [arena, level](const ast::Or &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::PublicScope &n) {
                    for (const auto &statement : n.body) {
                        print_node(arena, statement, level + 1);
                    }
                },
                [arena, level](const ast::Root &n) {
                    for (const auto &statement : n.body) {
                        print_node(arena, statement, level + 1);
                    }
                },
                [arena, level](const ast::Sub &n) {
                    print_node(arena, n.lhs, level + 1);
                    print_node(arena, n.rhs, level + 1);
                },
                [arena, level](const ast::UseDeclaration &n) {
                    for (const auto &segment : n.name_segments) {
                        print_node(arena, segment, level + 1);
                    }
                },
                [arena, level](const ast::VariableDeclaration &n) {
                    print_node(arena, n.name, level + 1);
                    if (n.type_notation) {
                        print_node(arena, *n.type_notation, level + 1);
                    }
                    if (n.expression) {
                        print_node(arena, *n.expression, level + 1);
                    }
                },
                [arena, level](const ast::WhileLoop &n) {
                    print_node(arena, n.cond, level + 1);
                    for (const auto &statement : n.body) {
                        print_node(arena, statement, level + 1);
                    }
                },
                [](const auto &) { std::unreachable(); } },
            arena.get(ni));
    }
} // namespace fla::compiler
