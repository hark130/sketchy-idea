/*
 *  Source file to highlight the Resource Acquisition Is Initialization (RAII) style auto-cleanup
 *  macros in the skid_memory header.
 *
 *  Copy/paste the following...

./code/dist/test_sm_raii_void_macro.bin 0090
CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all ./code/dist/test_sm_raii_void_macro.bin 0090

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
    int retval = ENOERR;                        // Errno values from execution (DO NOT CALL exit()!)
    SKID_AUTO_FREE_VOID void *raii_buf = NULL;  // RAII buffer variable

    // INPUT VALIDATION
    if (argc != 2)
    {
       retval = EINVAL;
    }

    // DO IT
    // Allocate memory
    if (ENOERR == retval)
    {
        raii_buf = alloc_skid_mem(1, sizeof(int), &retval);
    }
    // Convert it
    if (ENOERR == retval)
    {
        errno = ENOERR;
        *(int *)raii_buf = atoi(argv[1]);
        retval = errno;
        if (0 == *(int *)raii_buf && ENOERR != retval)
        {
            FPRINTF_ERR("%s The call to atoi(%s) failed\n", DEBUG_INFO_STR, argv[1]);
            PRINT_ERRNO(retval);
        }
    }
    // Print it
    if (ENOERR == retval)
    {
        printf("Read the string '%s' which was converted to the integer '%d'\n",
               argv[1], *(int *)raii_buf);
    }

    // CLEANUP
    // Not necessary!

    // DONE
    if (ENOERR != retval)
    {
       print_usage(argv[0]);
    }
    return retval;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <INTEGER>\n", prog_name);
}
