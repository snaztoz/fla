#ifndef FLA_COMPILER_UTIL_H
#define FLA_COMPILER_UTIL_H

#include <expected>
#include <filesystem>

namespace fla::compiler::util
{
    template <class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };

    const std::expected<std::string, std::string> read_file(const std::filesystem::path path);
} // namespace fla::compiler::util

#endif
