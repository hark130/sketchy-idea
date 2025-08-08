/*
 *  Source file to highlight the packed struct macro in the skid_macros header.
 *
 *  Copy/paste the following...

./code/dist/test_misc_packed_macro.bin ./devops/files/hide_and_seek_champ.bmp

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
    int retval = ENOERR;  // Errno values from execution
    // char *bmp_file = NULL;  // BMP File

    // INPUT VALIDATION
    if (argc != 2)
    {
       retval = EINVAL;
    }

    // DO IT
    // Copy it
    if (ENOERR == retval)
    {
        // TD: DDN...
        retval = ENOSYS;  // ...implement this functionality in SKID-23
    }

    // DONE
    if (ENOERR != retval)
    {
       print_usage(argv[0]);
    }
    return retval;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <BMP FILENAME>\n", prog_name);
}
