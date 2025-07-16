/*
 *  Manually test skid_assembly's inline assembly functionality.
 *  This binary makes use of call_write() to write to stdout.
 *
 *  Copy/paste the following...

./code/dist/test_sa_call_write.bin

 *
 */

#define SKID_DEBUG                  // Enable DEBUG logging

#include <errno.h>                  // EINVAL
#include <stdio.h>                  // fprintf()
#include <stdlib.h>                 // exit()
#include "skid_assembly.h"          // call_write()
#include "skid_debug.h"             // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_descriptors.h"  // write_fd()
#include "skid_macros.h"            // ENOERR

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define MESSAGE "Hello from test_sa_call_write\n"

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;                          // Errno values
    ssize_t num_bytes = -1;                          // Number of bytes written
    char buf[] = { MESSAGE };                        // String to write
    size_t buf_size = strlen(buf) * sizeof(buf[0]);  // Byte size of buf's printable characters

    // INPUT VALIDATION
    if (argc != 1)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }

    // WRITE IT
    if (ENOERR == exit_code)
    {
        // num_bytes = fprintf(stdout, "%s", buf);                      // 1
        // exit_code = write_fd(SKID_STDOUT_FD, buf);                   // 2
        // if (ENOERR == exit_code) { num_bytes = buf_size; };          // 2
        num_bytes = call_write(SKID_STDOUT_FD, buf, buf_size);       // 3
        if (num_bytes != buf_size) { PRINT_WARNG(Partial write); };  // 3
    }
    if (ENOERR == exit_code)
    {
        printf("\n%s wrote %zu-of-%zu bytes\n", argv[0], num_bytes, strlen(buf));
    }

    // DONE
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s\n", prog_name);
}
