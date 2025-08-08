/*
 *  This library defines select()-related functionality.
 */

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_file_descriptors.h"          // read_fd()
#include "skid_macros.h"                    // ENOERR, SKID_INTERNAL, SKID_STDIN_FD
#include "skid_validation.h"                // validate_skid_err(), validate_skid_fd()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Validate an fd_set pointer on behalf of the library.
 *
 *  Args:
 *      fds: The fd_set to validate.
 *      optional: If false, the pointer must be valid.  If true, NULL pointers are ignored.
 *
 *  Returns:
 *      ENOERR on success, EINVAL on failed validation.
 */
SKID_INTERNAL int validate_skid_fd_set(fd_set *fds, bool optional);

/*
 *  Description:
 *      Validate (some) call_select() arguments on behalf of the library.
 *
 *  Args:
 *      nfds: This argument should be set to the highest-numbered file descriptor in any of
 *          the three sets, plus 1.  Validated against SKID_STDIN_FD + 1 for the minimum value.
 *      readfds: [Optional] The file descriptors in this set are watched to see if they are
 *          ready for reading.  May be NULL (but between readfds, writefds, and exceptfds... one
 *          of them must be a valid pointer).
 *      writefds: [Optional] The file descriptors in this set are watched to see if they are
 *          ready for writing.  May be NULL (but between readfds, writefds, and exceptfds... one
 *          of them must be a valid pointer).
 *      exceptfds: [Optional] The file descriptors in this set are watched for
 *          "exceptional conditions".  May be NULL (but between readfds, writefds, and
 *          exceptfds... one of them must be a valid pointer).
 *      errnum: Storage location for errno values encountered.
 *
 *  Returns:
 *      ENOERR on success, EINVAL on failed validation.
 */
SKID_INTERNAL int validate_skid_select_args(int nfds, fd_set *readfds, fd_set *writefds,
                                            fd_set *exceptfds, int *errnum);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int add_fd_to_set(int fd, fd_set *dstfds)
{
    // LOCAL VARIABLES
    int results = validate_skid_fd(fd);  // Store errno value

    // INPUT VALIDATION
    if (ENOERR == results)
    {
        results = validate_skid_fd_set(dstfds, false);
    }

    // ADD IT
    if (ENOERR == results)
    {
        // void FD_SET(int fd, fd_set *set);
        FD_SET(fd, dstfds);
    }

    // DONE
    return results;
}


int call_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                struct timeval *timeout, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;  // Store errno value
    int num_rdy = 0;       // Number of fds elements that are ready (before the timeout expires)

    // INPUT VALIDATION
    results = validate_skid_select_args(nfds, readfds, writefds, exceptfds, errnum);

    // CALL IT
    if (ENOERR == results)
    {
        num_rdy = select(nfds, readfds, writefds, exceptfds, timeout);
        if (0 > num_rdy)
        {
            results = errno;
            PRINT_ERROR(The call to select() failed);
        }
        else if (0 == num_rdy)
        {
            PRINT_WARNG(The call to select() timed out before any file descriptors became ready);
        }
        else
        {
            FPRINTF_ERR("%s %d file descriptors are ready\n", DEBUG_INFO_STR, num_rdy);
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


int clear_fd_set(fd_set *oldfds)
{
    // LOCAL VARIABLES
    int results = validate_skid_fd_set(oldfds, false);  // Store errno value

    // CLEAR IT
    if (ENOERR == results)
    {
        // void FD_ZERO(fd_set *set);
        FD_ZERO(oldfds);
    }

    // DONE
    return results;
}


int copy_fd_set(fd_set *srcfds, fd_set *dstfds)
{
    // LOCAL VARIABLES
    int results = validate_skid_fd_set(srcfds, false);  // Store errno value

    // INPUT VALIDATION
    if (ENOERR == results)
    {
        results = validate_skid_fd_set(dstfds, false);
    }

    // COPY IT
    if (ENOERR == results)
    {
        *dstfds = *srcfds;  // Surely, it's not this easy?!
    }

    // DONE
    return results;
}


int initialize_fd_set(int *fds, int num_fds, fd_set *newfds)
{
    // LOCAL VARIABLES
    int results = validate_skid_fd_set(newfds, false);  // Store errno value

    // INPUT VALIDATION
    if (ENOERR == results)
    {
        if (NULL == fds)
        {
            results = EINVAL;
        }
        else if (num_fds < 1)
        {
            results = ERANGE;
        }
        else
        {
            for (int i = 0; i < num_fds; i++)
            {
                results = validate_skid_fd(*(fds + i));
                if (ENOERR != results)
                {
                    break;  // We found a bad file descriptor
                }
            }
        }
    }

    // INITIALIZE IT
    // Clear it
    if (ENOERR == results)
    {
        results = clear_fd_set(newfds);
    }
    // Copy it
    if (ENOERR == results)
    {
        for (int i = 0; i < num_fds; i++)
        {
            results = add_fd_to_set(*(fds + i), newfds);
            if (ENOERR != results)
            {
                break;  // Error?!  This far in?!
            }
        }
    }

    // DONE
    if (ENOERR != results)
    {
        clear_fd_set(newfds);  // Best effort
    }
    return results;
}


bool is_fd_in_set(int fd, fd_set *haystackfds, int *errnum)
{
    // LOCAL VARIABLES
    int results = validate_skid_fd(fd);  // Store errno value
    bool is_it = false;                  // Is fd in haystackfds?

    // INPUT VALIDATION
    if (ENOERR == results)
    {
        results = validate_skid_fd_set(haystackfds, false);
    }
    if (ENOERR == results)
    {
        results = validate_skid_err(errnum);
    }

    // IS IT?
    if (ENOERR == results)
    {
        // int FD_ISSET(int fd, fd_set *set);
        if (0 != FD_ISSET(fd, haystackfds))
        {
            is_it = true;
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return is_it;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL int validate_skid_fd_set(fd_set *fds, bool optional)
{
    // LOCAL VARIABLES
    int results = ENOERR;  // Store errno value

    // INPUT VALIDATION
    if (NULL == fds)
    {
        if (false == optional)
        {
            results = EINVAL;  // Suffer not the NULL pointer
        }
    }

    // DONE
    return results;
}


SKID_INTERNAL int validate_skid_select_args(int nfds, fd_set *readfds, fd_set *writefds,
                                            fd_set *exceptfds, int *errnum)
{
    // LOCAL VARIABLES
    int results = validate_skid_err(errnum);  // Store errno value

    // INPUT VALIDATION
    if (ENOERR == results)
    {
        if (nfds < (SKID_STDIN_FD + 1))
        {
            results = EINVAL;
            PRINT_ERROR(The nfds argument must be positive);
        }
        else if (NULL == readfds && NULL == writefds && NULL == exceptfds)
        {
            results = EINVAL;
            PRINT_ERROR(At least one of the descriptor sets must be a valid pointer);
        }
    }
    if (ENOERR == results)
    {
        results = validate_skid_fd_set(readfds, true);  // Optional
    }
    if (ENOERR == results)
    {
        results = validate_skid_fd_set(writefds, true);  // Optional
    }
    if (ENOERR == results)
    {
        results = validate_skid_fd_set(exceptfds, true);  // Optional
    }

    // DONE
    if (ENOERR != results)
    {
        PRINT_ERRNO(results);
    }
    return results;
}
