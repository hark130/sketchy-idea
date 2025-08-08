/*
 *  This source file was created for the express purpose of showcasing gcc options.
 *  It explicitly defines its own custom entry point.  However, it still makes use of linked
 *  libraries.
 *
 *  Copy/paste the following...

gcc -nostartfiles -o ./code/dist/test_gcc_nostartfiles.bin ./code/test/test_gcc_nostartfiles.c ./code/src/skid_assembly.c -I ./code/include && \
./code/dist/test_gcc_nostartfiles.bin

 *
 */

#ifdef SKID_DEBUG
// Explicitly disable DEBUG logging since I don't have runtime support for skid_debug function calls like printf()
#undef SKID_DEBUG
#endif /* SKID_DEBUG */

#include <stddef.h>                     // size_t
#include <stdio.h>                      // printf()
#include <stdlib.h>                     // exit()
#include <string.h>                     // strlen()
#include <unistd.h>                     // ssize_t
#include "skid_assembly.h"              // call_exit(), call_write()

#define MESSAGE "Hello from test_gcc_nostartfiles.bin\n"


void _start()
{
    char buf[] = { MESSAGE };  // String to write
    call_write(1, buf, sizeof(buf) - 1);  // -1 for the nul character
    call_exit(0);
}
