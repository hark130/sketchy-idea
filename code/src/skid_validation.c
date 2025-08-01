/*
 *  This library defines functionality to help automate validation.
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <stddef.h>                         // NULL
#include <string.h>                         // strlen()
#include <sys/stat.h>                       // lstat(), struct stat
#include "skid_debug.h"                     // MODULE_*LOAD(), *PRINT_*()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_validation.h"                // ENOERR

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
        FPRINTF_ERR("%s - File descriptor %d failed validation\n", DEBUG_ERROR_STR, fd);
        PRINT_ERRNO(result);
    }

    // DONE
    return result;
}


int validate_skid_pathname(const char *pathname, bool must_exist)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Validation result

    // VALIDATE IT
    // pathname
    if (NULL == pathname)
    {
        result = EINVAL;  // Invalid argument
        PRINT_ERROR(Invalid Argument - Received a null pathname pointer);
    }
    else if (0 == strlen(pathname))
    {
        result = EINVAL;  // Invalid argument
        PRINT_ERROR(Invalid Argument - Received an empty pathname);
    }
    else if (true == must_exist && false == is_path(pathname, &result))
    {
        if (ENOERR == result)
        {
            result = ENOENT;  // No error, file was just missing
        }
        FPRINTF_ERR("%s pathname %s failed validation", DEBUG_ERROR_STR, pathname);
        PRINT_ERRNO(result);
    }

    // DONE
    return result;
}


int validate_skid_shared_name(const char *shared_name, bool must_port)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values

    // INPUT VALIDATION
    result = validate_skid_string(shared_name, false);
    if (ENOERR == result)
    {
        if (true == must_port && '/' != shared_name[0])
        {
            result = EINVAL;
            FPRINTF_ERR("%s The shared object name '%s' must begin with a '/' (for portability)\n",
                        DEBUG_ERROR_STR, shared_name);
        }
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
