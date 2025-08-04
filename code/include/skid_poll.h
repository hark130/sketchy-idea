#ifndef __SKID_POLL__
#define __SKID_POLL__


#define SKID_BAD_TIME_T ((time_t)-1)

#include <poll.h>                           // struct pollfd


/*
 *  Description:
 *      Wait for one of a set of file descriptors to become ready to perform I/O by calling poll().
 *
 *  Args:
 *      fds: An array of structs specifying the filed descriptors to be monitored.
 *      nfds: The number of items in the fds array.
 *      timeout: The number of milliseconds that poll() should block waiting for a file
 *          descriptor to become ready.  The call will block until: an fd becomes ready, the call
 *          is interrupted by a signal handler, or the timeout expires.  Specifying a negative
 *          value in timeout means an infinite timeout.  Specifying a timeout of zero causes
 *          poll() to return immediately, even if no file descriptors are ready.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      The return value from poll(), interpreted as follows:
 *      - A nonnegative value indicates the number of elements in pollfds whose revents
 *        fields have been set to a nonzero value (indicating an event or an error).
 *      - A return value of zero indicates that the system call timed out before any file
 *        descriptors became read.
 *      - On error, -1 is returned, and errnum is set with the errno value.
 */
int call_poll(struct pollfd *fds, nfds_t nfds, int timeout, int *errnum);

/*
 *  Description:
 *      Process one pollfd struct.  If there is data to read (e.g., POLLIN), read it.  If there
 *      is an error (e.g., POLLERR), close the fd and set it to SKID_BAD_FD.  POLLHUP conditions
 *      will still attempt to read (in case of a pipe or stream socket).  The fd will be closed
 *      regardless of a successful read or not.
 *
 *  Args:
 *      poll_fd: A pointer to the pollfd struct to potentially read from.
 *      revents: [Out] The revents value from poll_fd.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      If there was data to read, a pointer to a heap-allocated array containing the data
 *      read from the file descriptor.  Otherwise, NULL.  The revents argument will always be
 *      set to match poll_fd->revents and errnum will always be set with any errno values
 *      encountered.  The errno value EBADF will be used if poll_fd's fd is already set
 *      to SKID_BAD_FD.
 */
char *read_pollfd(struct pollfd *poll_fd, int *revents, int *errnum);


#endif  /* __SKID_POLL__ */
