/*
 *  This library defines functionality to help automate validation.
 */

// #define SKID_DEBUG                        // Enable DEBUG logging

#include "skid_debug.h"                     // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_validation.h"                // ENOERR
#include <errno.h>                          // EINVAL
#include <stddef.h>                         // NULL
#include <sys/stat.h>                       // lstat(), struct stat

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int validate_skid_err(int *err)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // The results of validation

    // INPUT VALIDATION
    if (!err)
    {
        result = EINVAL;  // NULL pointer
    }

    // DONE
    return result;
}


int validate_skid_fd(int fd)
{
    // LOCAL VARIABLES
    int result = EBADF;  // Validation result

    // INPUT VALIDATION
    if (fd >= 0 && SKID_BAD_FD != fd)
    {
        result = ENOERR;  // Good(?).
    }
    else
    {
        FPRINTF_ERR("%s file descriptor %d failed validation", DEBUG_ERROR_STR, fd);
        PRINT_ERRNO(result);
    }

    // DONE
    return result;
}


int validate_skid_pathname(const char *pathname, bool must_exist)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Validation result
    struct stat sb;       // Out-parameter for the call to lstat()

    // VALIDATE IT
    // pathname
    if (!pathname)
    {
        result = EINVAL;  // Invalid argument
        PRINT_ERROR(Invalid Argument - Received a null pathname pointer);
    }
    else if (!(*pathname))
    {
        result = EINVAL;  // Invalid argument
        PRINT_ERROR(Invalid Argument - Received an empty pathname);
    }
    else if (true == must_exist && -1 == lstat(pathname, &sb))
    {
        result = errno;
        if (ENOERR == result)
        {
            result = EINVAL;  // Unspecified error
            PRINT_ERROR(Unspecified error);
        }
        FPRINTF_ERR("%s pathname %s failed validation", DEBUG_ERROR_STR, pathname);
        PRINT_ERRNO(result);
    }

    // DONE
    return result;
}


int validate_skid_sockfd(int sockfd)
{
    // LOCAL VARIABLES
    int result = EBADF;  // Validation result

    // INPUT VALIDATION
    if (sockfd >= 0 && SKID_BAD_FD != sockfd)
    {
        result = ENOERR;  // Good(?).
    }

    // DONE
    return result;
}


int validate_skid_string(const char *string, bool can_be_empty)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Validation result

    // INPUT VALIDATION
    if (NULL == string)
    {
        result = EINVAL;  // NULL pointer
    }
    else if (false == can_be_empty)
    {
        if (0x0 == *string)
        {
            result = EINVAL;  // Empty string
        }
    }

    // DONE
    return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
