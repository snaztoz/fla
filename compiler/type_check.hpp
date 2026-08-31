#ifndef FLA_COMPILER_TYPE_CHECK_H
#define FLA_COMPILER_TYPE_CHECK_H

#include <expected>

#include "ast.hpp"
#include "context.hpp"
#include "error.hpp"

namespace fla::compiler::type_check
{
    using Result = std::expected<Namespace, Error>;

    const Result read_declarations(const ast::Arena &, const ast::NodeIndex);
} // namespace fla::compiler::type_check

#endif
