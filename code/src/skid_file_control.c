/*
 *      This library defines functionality to read and control file descriptors.
 */

#include <errno.h>                      // errno
#include <fcntl.h>                      // fcntl(), FD_CLOEXEC
#include <stdarg.h>                     // va_end(), va_list, va_start()
#include "skid_debug.h"                 // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_control.h"          // get_read_lock(), get_write_lock()
#include "skid_file_descriptors.h"      // read_fd(), write_fd()
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
 *      Standardize the way this library calls fcntl(), with flock structs, and responds to errors.
 *      This function always uses the same whence (SEEK_SET), start (0), and len (0) for the
 *      flock struct to represent the entirety of the file descriptor.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *      fd: Open file descriptor to pass to fcntl().
 *      cmd: The fnctl() operation to execute.  Must be a command that utilizes a struct flock
 *          (e.g., F_SETLK, F_SETLKW, F_GETLK).  See: fcntl(2).
 *      lock_type: The lock type to set in the flock struct.  (e.g., F_RDLCK, F_WRLCK, F_UNLCK)
 *          See: fcntl(2).
 *
 *  Returns:
 *      For a successful call, the return value depends on the operation.  See: fcntl(2).
 *      On error, -1 is returned, and errnum is set appropriately.  The errno value
 *      EOPNOTSUPP is used to indicate an unsupported cmd or lock_type.
 */
int call_fcntl_flock(int *errnum, int fd, int cmd, short lock_type);

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


int get_read_lock(int fd)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values
    int retval = -1;      // Return value from fcntl()

    // GET IT
    retval = call_fcntl_flock(&result, fd, F_SETLK, F_RDLCK);
    if (-1 == retval)
    {
        FPRINTF_ERR("%s Failed to get a read lock on file descriptor '%d'\n", DEBUG_WARNG_STR, fd);
    }

    // DONE
    return result;
}


int get_write_lock(int fd)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values
    int retval = -1;      // Return value from fcntl()

    // GET IT
    retval = call_fcntl_flock(&result, fd, F_SETLK, F_RDLCK);
    if (-1 == retval)
    {
        FPRINTF_ERR("%s Failed to get a write lock on file descriptor '%d'\n", DEBUG_WARNG_STR, fd);
    }

    // DONE
    return result;
}


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


char *read_locked_fd(int fd, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Errno values
    int tmp_result = ENOERR;  // More errno values
    char *fd_cont = NULL;     // Content read from fd

    // DO IT
    // Lock the file descriptor
    result = get_read_lock(fd);
    // Read the file descriptor
    if (ENOERR == result)
    {
        fd_cont = read_fd(fd, &result);
        // Release the lock (regardless of read_fd()'s success)
        tmp_result = release_lock(fd);
        if (ENOERR == result)
        {
            result = tmp_result;  // Only report the first errno value
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return fd_cont;
}


int release_lock(int fd)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values
    int retval = -1;      // Return value from fcntl()

    // GET IT
    retval = call_fcntl_flock(&result, fd, F_SETLK, F_UNLCK);
    if (-1 == retval)
    {
        FPRINTF_ERR("%s Failed to release a lock on file descriptor '%d'\n", DEBUG_WARNG_STR, fd);
    }

    // DONE
    return result;
}


int write_locked_fd(int fd, const char *msg)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Errno values
    int tmp_result = ENOERR;  // More errno values

    // DO IT
    // Lock the file descriptor
    result = get_write_lock(fd);
    // Write to the file descriptor
    if (ENOERR == result)
    {
        result = write_fd(fd, msg);
        // Release the lock (regardless of write_fd()'s success)
        tmp_result = release_lock(fd);
        if (ENOERR == result)
        {
            result = tmp_result;  // Only report the first errno value
        }
    }

    // DONE
    return result;
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


int call_fcntl_flock(int *errnum, int fd, int cmd, short lock_type)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values
    int retval = -1;      // Return value from fcntl()
    // Flock struct
    struct flock fl = {
        .l_type = lock_type,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };

    // INPUT VALIDATION
    if (F_SETLK != cmd && F_SETLKW != cmd && F_GETLK != cmd)
    {
        result = EOPNOTSUPP;  // Unsupported cmd
    }
    else if (F_RDLCK != lock_type && F_WRLCK != lock_type && F_UNLCK != lock_type)
    {
        result = EOPNOTSUPP;  // Unsupported lock_type
    }
    // NOTE: call_fcntl() validates all other input

    // CALL IT
    if (ENOERR == result)
    {
        retval = call_fcntl(&result, fd, cmd, &fl);
    }

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
