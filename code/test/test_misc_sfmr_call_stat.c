/*
 *  Manually test the skid_file_metadata_read library's ability to hide the visibility of
 *  internal functions.  This manual test code attempts to call call_stat(), the internal
 *  function that underpins much of skid_file_metadata_read's API functions.
 *
 *  The use case in this particular manual test file is:
 *  1. Reverse engineering (RE) has turned up a function, internal to a SKID library, that I want to
 *     call (e.g., it's unsafe and I want to exploit it).
 *  2. RE has provided me the function declaration which I've added to my code (to circumvent
 *     "implicit function declaration" warnings)
 *  3. I compile my code and link against the SKID shared object.
 *
 *  The theory is, my code won't work when the gcc attribute is restricting access.
 *
 *  Copy/paste the following...

# 1. Prepare
make && make install && \

# 2. Prove the code works when statically compiled against source
./code/dist/test_misc_sfmr_call_stat.bin ./code/test/test_input/regular_file.txt && \
./code/dist/test_misc_sfmr_call_stat.bin ./code/test/test_input/ && \

# 3. Prove the linker fails when pointed at the SKID shared object
gcc -o ./code/dist/test_libskid_sfmr_call_stat.bin ./code/test/test_misc_sfmr_call_stat.c -lsketchyidea && \
./code/dist/test_misc_sfmr_call_stat.bin ./code/test/test_input/regular_file.txt && \
./code/dist/test_misc_sfmr_call_stat.bin ./code/test/test_input/ && \

# 4. Trailing do-nothing command (for a partial copy/paste)
echo

 *
 */

// Standard includes
#include <errno.h>                    // EINVAL
#include <stdint.h>                   // uintmax_t
#include <stdio.h>                    // fprintf()
#include <stdlib.h>                   // exit()
#include <sys/stat.h>                 // mode_t, struct stat
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
// #include "skid_file_metadata_read.h"  // call_stat()


/*
 *  A copy/paste of the skid_file_metadata_read's internal declaration to circumvent "implicit 
 *  function declaration" warnings.
 */
int call_stat(const char *pathname, struct stat *statbuf, int *errnum);


/*
 *  A copy/paste of skid_file_metadata_read's get_file_perms() implementation.
 *  Input validation removed (to avoid any other linking requirements), SKID macro references
 *  removed (e.g., ENOERR), and the name was mangled to avoid compiler errors (e.g., name
 *  collisions).
 */
mode_t get_those_file_perms(const char *pathname, int *errnum);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = 0;      // Store errno and/or results here
    mode_t answer = 0;      // Return value from get_file_perms()
    char *pathname = NULL;  // Get this from argv[1]

    // INPUT VALIDATION
    if (argc != 2)
    {
       fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
       exit_code = EINVAL;
    }
    else
    {
        pathname = argv[1];
    }

    // CHECK IT
    if (!exit_code)
    {
        answer = get_those_file_perms(pathname, &exit_code);
        if (exit_code)
        {
            PRINT_ERROR(The call to get_those_file_perms() failed);
            PRINT_ERRNO(exit_code);
        }
        else
        {
            printf("%s has the following permissions: %jo (octal).\n", pathname, (uintmax_t)answer);
        }
    }

    // DONE
    exit(exit_code);
}


mode_t get_those_file_perms(const char *pathname, int *errnum)
{
    // LOCAL VARIABLES
    mode_t perm_mask = S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO;  // Perm bitmask
    mode_t retval = 0;                                                             // File perms
    int err = 0;                                                                   // Errno value
    struct stat stat_struct;                                                       // stat struct

    // GET IT
    // Fetch metadata
    if (0 == err)
    {
        err = call_stat(pathname, &stat_struct, errnum);
    }
    // Get it
    if (0 == err)
    {
        retval = stat_struct.st_mode & perm_mask;
    }

    // DONE
    return retval;
}
