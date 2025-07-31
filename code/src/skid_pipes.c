/*
 *  This library defines functionality to help manage pipes.
 */


#include <errno.h>                          // errno, EEXIST
#include <stdbool.h>                        // false
#include "skid_debug.h"                     // PRINT_E*(), MODULE_*LOAD()
#include "skid_file_descriptors.h"          // close_fd()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_file_operations.h"           // delete_file()
#include "skid_validation.h"                // validate_skid_err(), validate_skid_pathname()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int close_pipe(int *pipefd, bool quiet)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values

    // INPUT VALIDATION
    // Handled by close_fd()

    // CLOSE IT
    result = close_fd(pipefd, quiet);

    // DONE
    return result;
}


int delete_named_pipe(const char *pathname)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values

    // INPUT VALIDATION
    // Handled by delete_file()

    // CLOSE IT
    result = delete_file(pathname);

    // DONE
    return result;
}


int make_named_pipe(const char *pathname, mode_t mode)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution

    // INPUT VALIDATION
    result = validate_skid_pathname(pathname, false);

    // ENVIRONMENT VALIDATION
    if (ENOERR == result)
    {
        if (true == is_path(pathname, &result))
        {
            result = EEXIST;  // We will not overwrite it
        }
        else
        {
            result = ENOERR;  // Ignore other errors
        }
    }

    // MAKE IT
    if (ENOERR == result)
    {
        if (mkfifo(pathname, mode))
        {
            result = errno;
            PRINT_ERROR(The call to mkfifo() failed);
            if (ENOERR == result)
            {
                PRINT_ERROR(Unspecified error replaced with broken pipe errno);
                result = EPIPE;  // Unspecified error
            }
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
