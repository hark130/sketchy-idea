#ifndef __SKID_SELECT__
#define __SKID_SELECT__

#include <stdbool.h>                        // bool
#include <sys/select.h>                     // fd_set

/*
 *  Description:
 *      Add fd to the dstfds set.
 *
 *  Args:
 *      fd: The file descriptor to add to dstfds.
 *      dstfds: [Out] The fd_set to to add fd to.
 *
 *  Returns:
 *      ENOERR on success, errno on error.
 */
int add_fd_to_set(int fd, fd_set *dstfds);

/*
 *  Description:
 *      Monitor multiple file descriptors, waiting until one or more of the file descriptors
 *      become "ready" for some class of I/O operation by calling select().  A file
 *      descriptor is considered ready if it is possible to perform a corresponding I/O
 *      operation (e.g., read(2), or a sufficiently small write(2)) without blocking.
 *
 *  Note:
 *      The readfds, writefds, and execptfds arguments may all be optional but at least one
 *      of them must be a valid pointer.
 *
 *  Warning:
 *      select() can monitor only file descriptors numbers that are less than FD_SETSIZE 
 *      and this limitation will not change.  (see: select(2))
 *
 *  Args:
 *      nfds: This argument should be set to the highest-numbered file descriptor in any of
 *          the three sets, plus 1.  The indicated file descriptors in each set are checked,
 *          up to this limit.
 *      readfds: [Optional In/Out] The file descriptors in this set are watched to see if they are
 *          ready for reading.  After select() has returned, readfds will be cleared of all
 *          file descriptors except for those that are ready for reading.
 *      writefds: [Optional In/Out] The file descriptors in this set are watched to see if they are
 *          ready for writing.  After select() has returned, writefds will be cleared of all
 *          file descriptors except for those that are ready for writing.
 *      exceptfds: [Optional In/Out] The file descriptors in this set are watched for
 *          "exceptional conditions".  After select() has returned, exceptfds will be cleared of
 *          all file descriptors except for those for which an exceptional condition has occurred.
 *      timeout: [Optional In/Out] a timeval structure that specifies the interval that select()
 *          should block waiting for a file descriptor to become ready.  If both fields of the
 *          timeval structure are zero, then select() returns immediately.  If timeout is
 *          specified as NULL, select() blocks indefinitely waiting for a file descriptor
 *          to become ready.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      The return value from select(), interpreted as follows:
 *      - A nonnegative value indicates the number of file descriptors contained in the three
 *        returned descriptor sets.
 *      - A return value of zero indicates that the the timeout expired before any file
 *        descriptors became ready.
 *      - On error, -1 is returned, and errnum is set with the errno value.
 */
int call_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                struct timeval *timeout, int *errnum);

/*
 *  Description:
 *      Remove all file descriptors from oldfds by using FD_ZERO().
 *
 *  Args:
 *      oldfds: [Out] The fd_set to clear.
 *
 *  Returns:
 *      ENOERR on success, errno on error.
 */
int clear_fd_set(fd_set *oldfds);

/*
 *  Description:
 *      Remove all file descriptors from dstfds and add the srcfds to it.
 *
 *  Args:
 *      srcfds: The source fd_set to add to dstfds.
 *      dstfds: [Out] A fd_set to clear and add the srcfds to.
 *
 *  Returns:
 *      ENOERR on success, errno on error.
 */
int copy_fd_set(fd_set *srcfds, fd_set *dstfds);

/*
 *  Description:
 *      Remove all file descriptors from newfds and add num_fds number of file descriptors
 *      from the fds array into newfds.
 *
 *  Args:
 *      fds: An array of file descriptors to add to newfds.
 *      numfds: The number of file descripts in fds.
 *      newfds: [Out] A fd_set to clear and add file descriptors to.
 *
 *  Returns:
 *      ENOERR on success, errno on error.
 */
int initialize_fd_set(int *fds, int num_fds, fd_set *newfds);

/*
 *  Description:
 *      Is fd in haystackfds?
 *
 *  Args:
 *      fd: The file descriptor to check for.
 *      haystackfds: The fd_set to investigate.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      True on success, false if not (or on error).  On error, errnum is set with an errno value.
 */
bool is_fd_in_set(int fd, fd_set *haystackfds, int *errnum);


#endif  /* __SKID_SELECT__ */
