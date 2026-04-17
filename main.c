#include <stdio.h>

#include "compiler.h"

int main(int argc, char const *argv[])
{
    printf("Hello, World!\n");
    orchid_compile("  abc >= 150 package");
    return 0;
}
