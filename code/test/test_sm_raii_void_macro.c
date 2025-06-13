/*
 *  Source file to highlight the Resource Acquisition Is Initialization (RAII) style auto-cleanup
 *  macros in the skid_memory header.
 *
 *  Copy/paste the following...

./code/dist/test_sm_raii_void_macro.bin
CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all ./code/dist/test_sm_raii_void_macro.bin 90

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <stdlib.h>                         // atoi()
#include "skid_debug.h"                     // FPRINTF_ERR(), PRINT_ERRNO()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // alloc_skid_mem(), SKID_AUTO_FREE_VOID

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;                     // Errno values from execution
    SKID_AUTO_FREE_VOID void *raii_buf = NULL;  // RAII buffer variable

    // INPUT VALIDATION
    if (argc != 2)
    {
       exit_code = EINVAL;
    }

    // DO IT
    // Allocate memory
    if (ENOERR == exit_code)
    {
        raii_buf = alloc_skid_mem(1, sizeof(int), &exit_code);
    }
    // Convert it
    if (ENOERR == exit_code)
    {
        errno = ENOERR;
        *(int *)raii_buf = atoi(argv[1]);
        exit_code = errno;
        if (0 == *(int *)raii_buf && ENOERR != exit_code)
        {
            FPRINTF_ERR("%s The call to atoi(%s) failed\n", DEBUG_INFO_STR, argv[1]);
            PRINT_ERRNO(exit_code);
        }
    }
    // Print it
    if (ENOERR == exit_code)
    {
        printf("Read the string '%s' which was converted to the integer '%d'\n",
               argv[1], *(int *)raii_buf);
    }

    // CLEANUP
    // Not necessary!

    // DONE
    if (ENOERR != exit_code)
    {
       print_usage(argv[0]);
    }
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <INTEGER>\n", prog_name);
}
