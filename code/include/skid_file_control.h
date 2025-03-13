#ifndef __SKID_FILE_CONTROL__
#define __SKID_FILE_CONTROL__

#include <stdbool.h>    // bool, false, true

/*
 *  Description:
 *      Use fcntl() to obtain a read lock on the given file descriptor.  The read lock will
 *      encompass the file descriptor's entire contents.  Directly utilize fcntl(F_SETLK) for
 *      finer control (see: fcntl(2)).
 *
 *  Args:
 *      fd: File descriptor to obtain a lock for.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.
 */
int get_read_lock(int fd);

/*
 *  Description:
 *      Use fcntl() to obtain a write lock on the given file descriptor.  The write lock will
 *      encompass the file descriptor's entire contents.  Directly utilize fcntl(F_SETLK) for
 *      finer control (see: fcntl(2)).
 *
 *  Args:
 *      fd: File descriptor to obtain a lock for.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.
 */
int get_write_lock(int fd);

/*
 *  Description:
 *      Determine if the FD_CLOEXEC bit is set for a file descriptor using fcntl().
 *
 *  Args:
 *      fd: File descriptor to inspect.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      True if the FD_CLOEXEC, false otherwise.  Also, false on error and errnum is set.
 */
bool is_close_on_exec(int fd, int *errnum);

/*
 *  Description:
 *      Obtain a read lock on the file descriptor, read its contents into a
 *      heap-allocated buffer, and release the read lock.  It is the caller's
 *      responsibility to free the buffer with free_skid_mem().
 *
 *  Notes:
 *      - get_read_lock() is used to obtain the lock.
 *      - skid_file_descriptor's read_fd() is called to read the fd.
 *      - release_lock() is used to release the lock.
 *
 *  Args:
 *      fd: File descriptor to read from.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Pointer to the heap-allocated buffer, on success.  NULL on error (check errnum for details).
 */
char *read_locked_fd(int fd, int *errnum);

/*
 *  Description:
 *      Release all locks on the file descriptor utilizing fcntl().
 *
 *  Args:
 *      fd: File descriptor to release locks on.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.
 */
int release_lock(int fd);

/*
 *  Description:
 *      Obtain a write lock on the file descriptor, write the string, and release the write lock.
 *
 *  Notes:
 *      - get_write_lock() is used to obtain the lock.
 *      - skid_file_descriptor's write_fd() is called to read the fd.
 *      - release_lock() is used to release the lock.
 *
 *  Args:
 *      fd: File descriptor to write to.
 *      msg: The nul-terminated message to write to fd.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int write_locked_fd(int fd, const char *msg);

#endif  /* __SKID_FILE_CONTROL__ */
