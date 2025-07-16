/*
 *  This source file was created for the express purpose of showcasing gcc options.
 *  It explicitly defines its own custom entry point.  However, it still makes use of linked
 *  libraries.
 *
 *  Copy/paste the following...

gcc -nostartfiles -o ./code/dist/showcase_gcc_nostartfiles.bin ./code/test/showcase_gcc_nostartfiles.c -static && \
./code/dist/showcase_gcc_nostartfiles.bin

 *
 */

#include <stddef.h>                     // size_t
#include <stdio.h>                      // printf()
#include <stdlib.h>                     // exit()
#include <unistd.h>                     // ssize_t
#include "skid_assembly.h"              // call_write()

#define MESSAGE "Hello from showcase_gcc_nostartfiles.bin\n"


void _start()
{
    char buf[] = { MESSAGE };  // String to write
    ssize_t num_bytes = call_write(1, buf, sizeof(buf) - 1);  // -1 for the nul character
    printf("\n%s wrote %zu-of-%zu bytes\n", argv[0], num_bytes, strlen(buf));
    exit(0);
}
