#include "compiler.hpp"

int main(int argc, char const *argv[])
{
    orchid::compiler::compile("  abc >= 150 use");

    return 0;
}
