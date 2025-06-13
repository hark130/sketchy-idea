/*
 *  Source file to highlight the Resource Acquisition Is Initialization (RAII) style auto-cleanup
 *  macros in the skid_memory header.
 *
 *  Copy/paste the following...

./code/dist/test_sm_raii_string_macro.bin "This is my string.  There are many like it but this one is mine."
CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all ./code/dist/test_sm_raii_string_macro.bin "This is my string.  There are many like it but this one is mine."

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include "skid_debug.h"                     // FPRINTF_ERR(), PRINT_ERRNO()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // copy_skid_string(), SKID_AUTO_FREE_CHAR

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int retval = ENOERR;                        // Errno values from execution (DO NOT CALL exit()!)
    SKID_AUTO_FREE_CHAR char *raii_str = NULL;  // RAII string variable

    // INPUT VALIDATION
    if (argc != 2)
    {
       retval = EINVAL;
    }

    // DO IT
    // Copy it
    if (ENOERR == retval)
    {
        raii_str = copy_skid_string(argv[1], &retval);
    }
    // Print it
    if (ENOERR == retval)
    {
        printf("Read the string '%s' which was copied into heap-memory at address '%p'\n",
               raii_str, raii_str);
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
    fprintf(stderr, "Usage: %s <STRING>\n", prog_name);
}
