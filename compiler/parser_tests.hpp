#ifndef FLA_COMPILER_PARSER_TESTS_H
#define FLA_COMPILER_PARSER_TESTS_H

#include <string_view>

#include "parser.hpp"

inline bool is_parseable(std::string_view src)
{
    return !!fla::compiler::parse(src);
}

#endif
