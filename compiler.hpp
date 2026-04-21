#ifndef ORCHID_COMPILE_H
#define ORCHID_COMPILE_H

#include <string_view>

namespace orchid::compiler
{
    int compile(std::string_view src);
}

#endif
