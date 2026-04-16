#include <stdio.h>

#include "compile.h"

int main(int argc, char const *argv[])
{
    printf("Hello, World!\n");
    orchid_compile("         package hello.world");
    return 0;
}
