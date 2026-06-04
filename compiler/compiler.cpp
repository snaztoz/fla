#include <cstdlib>
#include <cstring>
#include <print>
#include <string>
#include <string_view>

#include "compiler.hpp"
#include "error.hpp"
#include "fla/compiler.h"
#include "parser.hpp"

extern "C" {
int fla_compile(const char *src, struct FlaCompilerError *err)
{
    try {
        const auto result { fla::compiler::compile(src) };
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
    void print_node(const Node &node, const int level)
    {
        const std::string indentation(level * 2, ' ');

        std::print("{}{}", indentation, node_type_string(node.type));

        if (const auto *string_val { std::get_if<std::string_view>(&node.value) }) {
            std::print(" -> {}", *string_val);
        } else if (const auto *int_val { std::get_if<int>(&node.value) }) {
            std::print(" -> {}", *int_val);
        } else if (const auto *bool_val { std::get_if<bool>(&node.value) }) {
            std::print(" -> {}", *bool_val);
        }

        std::print(" ({}:{}:{})\n", node.line, node.column, node.len);

        for (const auto &child : node.children) {
            print_node(child, level + 1);
        }
    }

    std::expected<void, Error> compile(const std::string_view src)
    {
        Parser parser { src };

        auto root { parser.parse() };
        if (!root) {
            return std::unexpected(root.error());
        }

        print_node(*root, 0);

        return {};
    }
} // namespace fla::compiler
