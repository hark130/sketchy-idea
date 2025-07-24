#ifndef __SKID_PIPES__
#define __SKID_PIPES__

#include <stdbool.h>                        // bool

/*
 *  Description:
 *      Close a pipe file descriptor and sets it to SKID_BAD_FD (if it was successfully closed).
 *
 *  Args:
 *      pipefd: [In/Out] A pointer to a pipe file descriptor to close.
 *      quiet: If true, silences all logging/debugging.
 *
 *  Returns:
 *      On success, ENOERR is returned.  On error, errno is returned.
 */
int close_pipe(int *pipefd, bool quiet);

/*
 *  Description:
 *      Delete a named pipe by calling unlink().
 *
 *  Notes:
 *      The name for it is removed but processes which have the object open may continue to use it.
 *
 *  Args:
 *      filename: Absolute or relative named socket to delete.
 *
 *  Returns:
 *      On success, ENOERR is returned.  On error, errno is returned.
 */
int delete_named_pipe(const char *pathname);

/*
 *  Description:
 *      Make a FIFO special file with name pathname and permissions mode and open it.
 *
 *  Args:
 *      pathname: Absolute or relative pathname for the new FIFO special file.
 *      mode: Specifies pathname's permissions.  See: skid_macros.h for mode macros.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.  Uses EEXIST if pathname exists and
 *      EPIPE for an unspecified error.
 */
int make_named_pipe(const char *pathname, mode_t mode);

#endif  /* __SKID_PIPES__ */
