#ifndef FLA_COMPILER_ERROR_H
#define FLA_COMPILER_ERROR_H

#include <cstddef>
#include <string>

#include "ast.hpp"

namespace fla::compiler
{
    struct Error {
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t col;
        const std::string msg;
    };
} // namespace fla::compiler

namespace fla::compiler::error
{
    inline const Error from_metadata(const ast::Metadata &meta, std::string msg)
    {
        return Error { meta.pos, meta.len, meta.line, meta.col, msg };
    }
} // namespace fla::compiler::error

#endif
