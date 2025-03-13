/*
 *      This library defines functionality to read and control file descriptors.
 */

#include <errno.h>                      // errno
#include <fcntl.h>                      // fcntl(), FD_CLOEXEC
#include <stdarg.h>                     // va_end(), va_list, va_start()
#include "skid_debug.h"                 // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"                // ENOERR, NULL
#include "skid_validation.h"            // validate_skid_fd(), validate_skid_err()


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Standardize the way this library calls fcntl() and responds to errors.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *      fd: Open file descriptor to pass to fcntl().
 *      cmd: The fnctl() operation to execute.  See: fcntl(2).
 *      ... Some fcntl() cmds take a third argument.  See: fcntl(2).
 *
 *  Returns:
 *      For a successful call, the return value depends on the operation.  See: fcntl(2).
 *      On error, -1 is returned, and errnum is set appropriately.
 */
int call_fcntl(int *errnum, int fd, int cmd, ... /* arg */ );

/*
 *  Description:
 *      Get the file descriptor flags using fnctl().
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *      fd: Open file descriptor to pass to fcntl().
 *
 *  Returns:
 *      On success, the value of file descriptor flags.  On error, -1 is returned,
 *      and errnum is set appropriately.
 */
int get_fd_flags(int *errnum, int fd);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


bool is_close_on_exec(int fd, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;         // Errno values
    int retval = -1;             // Return value from fcntl()
    bool close_on_exec = false;  // Is the FD_CLOEXEC bit set?

    // INPUT VALIDATION
    result = validate_skid_err(errnum);

    // IS IT?
    // Get the file descriptor flags
    if (ENOERR == result)
    {
        retval = get_fd_flags(&result, fd);
    }
    // Check the FD_CLOEXEC bit
    if (ENOERR == result)
    {
        if (FD_CLOEXEC == (FD_CLOEXEC & retval))
        {
            close_on_exec = true;
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return close_on_exec;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int call_fcntl(int *errnum, int fd, int cmd, ... /* arg */ )
{
    // LOCAL VARIABLES
    va_list arg_ptr;
    int result = ENOERR;  // Errno values
    int retval = -1;      // Return value from fcntl()

    // SETUP
    va_start(arg_ptr, cmd);

    // INPUT VALIDATION
    result = validate_skid_err(errnum);
    if (ENOERR == result)
    {
        result = validate_skid_fd(fd);
    }

    // CALL IT
    if (ENOERR == result)
    {
        retval = fcntl(fd, cmd, arg_ptr);
        if (-1 == retval)
        {
            result = errno;
            PRINT_ERROR(The call to fcntl() failed);
            PRINT_ERRNO(result);
        }
    }

    // CLEANUP
    va_end(arg_ptr);

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return retval;
}


int get_fd_flags(int *errnum, int fd)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values
    int retval = -1;      // Return value from fcntl()

    // GET IT
    retval = call_fcntl(&result, fd, F_GETFD);

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return retval;
}
