/*
 *    This library defines functionality to create links to Linux files.
 */

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // errno
#include <stdbool.h>                        // false
#include <unistd.h>                         // link(), symlink()
#include "skid_debug.h"                     // PRINT_ERRNO()
#include "skid_validation.h"                // validate_skid_pathname()


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Validates the pathname arguments on behalf of this library.
 *  Args:
 *      pathname: A non-NULL pointer to a non-empty string.
 *  Returns:
 *      An errno value indicating the results of validation.  ENOERR on successful validation.
 */
int validate_pathname(const char *pathname);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int create_hard_link(const char *source, const char *hard_link)
{
    // LOCAL VARIABLES
    int errnum = ENOERR;  // Results of the function call

    // INPUT VALIDATION
    errnum = validate_pathname(source);
    if (ENOERR == errnum)
    {
        errnum = validate_pathname(hard_link);
    }

    // CREATE IT
    if (ENOERR == errnum)
    {
        if (link(source, hard_link))
        {
            errnum = errno;
            PRINT_ERROR(The call to link() failed);
            PRINT_ERRNO(errnum);
        }
    }

    // DONE
    return errnum;
}


int create_sym_link(const char *source, const char *sym_link)
{
    // LOCAL VARIABLES
    int errnum = ENOERR;  // Results of the function call

    // INPUT VALIDATION
    errnum = validate_pathname(source);
    if (ENOERR == errnum)
    {
        errnum = validate_pathname(sym_link);
    }

    // CREATE IT
    if (ENOERR == errnum)
    {
        if (symlink(source, sym_link))
        {
            errnum = errno;
            PRINT_ERROR(The call to symlink() failed);
            PRINT_ERRNO(errnum);
        }
    }

    // DONE
    return errnum;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int validate_pathname(const char *pathname)
{
    return validate_skid_pathname(pathname, false);  // Refactored for backwards compatibility
}
