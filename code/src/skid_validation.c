/*
 *    This library defines functionality to help automate validation.
 */

// #define SKID_DEBUG                        // Enable DEBUG logging

#include "skid_debug.h"                      // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_validation.h"            // ENOERR
#include <errno.h>                        // EINVAL
#include <stddef.h>                        // NULL


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
