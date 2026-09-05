#ifndef FLA_COMPILER_ERROR_H
#define FLA_COMPILER_ERROR_H

#include <cstddef>
#include <string>

namespace fla::compiler::error
{
    struct Error {
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t col;
        const std::string msg;
    };
} // namespace fla::compiler::error

#endif
