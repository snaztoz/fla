#ifndef FLA_COMPILER_TYPE_CHECK_H
#define FLA_COMPILER_TYPE_CHECK_H

#include <expected>

#include "ast.hpp"
#include "error.hpp"
#include "namespace.hpp"

namespace fla::compiler::type_check
{
    using Result = std::expected<ns::Namespace, error::Error>;

    struct Context {
        const ast::Arena arena;
        const ast::Root root;
    };

    const Result read_declarations(const Context &);
} // namespace fla::compiler::type_check

#endif
