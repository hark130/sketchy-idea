/*
 *  This library defines poll()-related functionality.
 */

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <stdbool.h>                        // bool, false, true
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_file_descriptors.h"          // read_fd()
#include "skid_macros.h"                    // ENOERR, SKID_INTERNAL
#include "skid_poll.h"                      // struct pollfd
#include "skid_validation.h"                // validate_skid_err()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Determine if poll_fd is valid, error free, and has data to read.  Calls
 *      validate_skid_pollfd() before investigating the revents value.
 *
 *  Args:
 *      poll_fd: A pointer to the pollfd struct to determine if data is available.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      RETURN VALUE    ERRNUM VAL  EXPLANATION
 *      --------------------------------------------------------------------------------------------
 *      false           ENOERR      The arg is valid but no data to read
 *      false           errno       The arg is invalid
 *      true            ENOERR      The arg is valid and there's data to read
 */
SKID_INTERNAL bool has_data(struct pollfd *poll_fd, int *errnum);

/*
 *  Description:
 *      Determine if poll_fd is valid and error-free.  Calls validate_skid_pollfd() before
 *      investigating the revents value.  A revents value of POLLRDHUP, if defined and detected,
 *      will be treated as the arg is valid but reported an "error" so that the the fd may be
 *      closed.
 *
 *  Args:
 *      poll_fd: A pointer to the pollfd struct to validate.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      RETURN VALUE    ERRNUM VAL  EXPLANATION
 *      --------------------------------------------------------------------------------------------
 *      false           ENOERR      The arg is valid but the pollfd struct reports an error
 *      false           errno       The arg is invalid
 *      true            ENOERR      The arg is valid and error-free
 */
SKID_INTERNAL bool is_good(struct pollfd *poll_fd, int *errnum);

/*
 *  Description:
 *      Validate a pollfd struct pointer on behalf of skid_poll.
 *
 *  Args:
 *      poll_fd: A pointer to the pollfd struct to validate.
 *
 *  Returns:
 *      ENOERR on success, EINVAL for a NULL pointer, EBADF for a bad file descriptor.
 */
SKID_INTERNAL int validate_skid_pollfd(struct pollfd *poll_fd);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int call_poll(struct pollfd *fds, nfds_t nfds, int timeout, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;  // Store errno value
    int num_rdy = 0;       // Number of fds elements that are ready (before the timeout expires)

    // INPUT VALIDATION
    if (NULL == fds)
    {
        results = EINVAL;  // NULL pointer
        PRINT_ERROR(The fds argument may not be NULL);
    }
    if (ENOERR == results)
    {
        if (nfds < 1)
        {
            results = EINVAL;
            PRINT_ERROR(The nfds argument must be positive);
        }
    }

    // CALL IT
    if (ENOERR == results)
    {
        num_rdy = poll(fds, nfds, timeout);
        if (num_rdy < 0)
        {
            results = errno;
            PRINT_ERROR(The call to poll() failed);
        }
        else if (0 == num_rdy)
        {
            PRINT_WARNG(The call to poll() timed out before any file descriptors became ready);
        }
        else
        {
            FPRINTF_ERR("%s %d of %lu file descriptors are ready\n", DEBUG_INFO_STR, num_rdy, nfds);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    if (ENOERR != results)
    {
        PRINT_ERRNO(results);
    }
    return num_rdy;
}


char *read_pollfd(struct pollfd *poll_fd, int *revents, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;      // Store errno value
    int local_revents = 0;     // Local variable storing poll_fd->revents
    char *read_msg = NULL;     // The message read from a ready fd
    bool it_is_good = false;   // Is the pollfd struct valid?
    bool it_has_data = false;  // Is there data to be read?
    bool has_hup = false;      // Special case a POLLHUP

    // INPUT VALIDATION
    results = validate_skid_err(errnum);
    if (ENOERR == results)
    {
        if (NULL == revents)
        {
            results = EINVAL;  // Suffer not the NULL pointer
        }
    }

    // READ IT
    // Is it good?
    if (ENOERR == results)
    {
        it_is_good = is_good(poll_fd, &results);
        if (ENOERR != results)
        {
            PRINT_ERROR(The pollfd struct is not good);
            PRINT_ERRNO(results);
        }
        else
        {
            local_revents = poll_fd->revents;  // Store the revents as soon as appropriate
        }
    }
    // Does it have data?
    if (ENOERR == results)
    {
        if (true == it_is_good)
        {
            it_has_data = has_data(poll_fd, &results);
            if (ENOERR != results)
            {
                PRINT_ERROR(The call to has_data() has failed);
                PRINT_ERRNO(results);
            }
        }
        else
        {
            // POLLHUP?
            if (((local_revents & POLLHUP) == POLLHUP) && 0 != POLLHUP)
            {
                has_hup = true;  // We're going to read anyway
            }
        }
    }
    // Read it
    if (ENOERR == results)
    {
        if (true == it_has_data || true == has_hup)
        {
            read_msg = read_fd(poll_fd->fd, &results);
            if (ENOERR != results)
            {
                if (true == has_hup)
                {
                    results = ENOERR;  // POLLHUP meant there was a chance data existed
                }
                else
                {
                    PRINT_ERROR(The call to read_fd() has failed to read data);
                    PRINT_ERRNO(results);
                }
            }
        }
    }

    // CLOSE IT?
    if (ENOERR == results)
    {
        if (false == it_is_good || true == has_hup)  // Redundant, but safe(?).
        {
            results = close_fd(&(poll_fd->fd), false);
        }
    }

    // DONE
    if (NULL != revents)
    {
        *revents = local_revents;
    }
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return read_msg;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL bool has_data(struct pollfd *poll_fd, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;      // Store errno value
    bool it_has_data = false;  // Is this poll_fd valid, error-free, and has data?

    // INPUT VALIDATION
    results = validate_skid_pollfd(poll_fd);
    if (ENOERR == results)
    {
        results = validate_skid_err(errnum);
    }

    // HAS DATA?
    if (ENOERR == results)
    {
        if (((poll_fd->revents & POLLIN) == POLLIN) && 0 != POLLIN)
        {
            FPRINTF_ERR("%s This pollfd struct is reporting there is data to read\n",
                        DEBUG_INFO_STR);
            it_has_data = true;
        }
        if (((poll_fd->revents & POLLPRI) == POLLPRI) && 0 != POLLPRI)
        {
            FPRINTF_ERR("%s This pollfd struct is reporting there is urgent data available\n",
                        DEBUG_INFO_STR);
            it_has_data = true;
        }
        if (((poll_fd->revents & POLLHUP) == POLLHUP) && 0 != POLLHUP)
        {
            // When reading from a channel such as a pipe or a stream socket, this event merely
            // indicates that the peer closed its end of the channel.  Subsequent reads from
            // the channel will return 0 (EOF) only after all outstanding data in the channel has
            // been consumed.
            PRINT_WARNG(This pollfd struct is reporting a hang up but there may be data);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return it_has_data;
}


SKID_INTERNAL bool is_good(struct pollfd *poll_fd, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;    // Store errno value
    bool it_is_good = true;  // Is this poll_fd valid and error-free?

    // INPUT VALIDATION
    results = validate_skid_pollfd(poll_fd);
    if (ENOERR == results)
    {
        results = validate_skid_err(errnum);
    }

    // IS GOOD?
    if (ENOERR == results)
    {
        if (((poll_fd->revents & POLLERR) == POLLERR) && 0 != POLLERR)
        {
            PRINT_WARNG(This pollfd struct is reporting an error condition);
            it_is_good = false;
        }
        if (((poll_fd->revents & POLLHUP) == POLLHUP) && 0 != POLLHUP)
        {
            // When reading from a channel such as a pipe or a stream socket, this event merely
            // indicates that the peer closed its end of the channel.  Subsequent reads from
            // the channel will return 0 (EOF) only after all outstanding data in the channel has
            // been consumed.
            PRINT_WARNG(This pollfd struct is reporting a hang up);
            it_is_good = false;
        }
        if (((poll_fd->revents & POLLNVAL) == POLLNVAL) && 0 != POLLNVAL)
        {
            PRINT_WARNG(This pollfd struct is reporting an invalid request);
            it_is_good = false;
        }
#ifdef POLLRDHUP
        if (((poll_fd->revents & POLLRDHUP) == POLLRDHUP) && 0 != POLLRDHUP)
        {
            PRINT_WARNG(This pollfd struct is reporting a remote shutdown);
            it_is_good = false;
        }
#endif  /* POLLRDHUP */
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return it_is_good;
}


SKID_INTERNAL int validate_skid_pollfd(struct pollfd *poll_fd)
{
    // LOCAL VARIABLES
    int results = ENOERR;  // Store errno value

    // VALIDATE IT
    if (NULL == poll_fd)
    {
        results = EINVAL;  // NULL pointer
    }
    else
    {
        results = validate_skid_fd(poll_fd->fd);
    }

    // DONE
    return results;
}
