#ifndef __SKID_FILE_DESCRIPTORS__
#define __SKID_FILE_DESCRIPTORS__

#include <stdbool.h>            // bool, false, true
#include <stddef.h>             // size_t
#include <sys/types.h>          // mode_t
#include "skid_macros.h"        // SKID_BAD_FD

/*
 *  Description:
 *      Close a file descriptor and sets it to SKID_BAD_FD (if it was successfully closed).
 *
 *  Args:
 *      fdp: [In/Out] A pointer to a file descriptor to close.
 *      quiet: If true, silences all logging/debugging.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int close_fd(int *fdp, bool quiet);

/*
 *  Description:
 *      Standardize how the dup2() system call is made.  The oldfd will be copied to the newfd.
 *      If the newfd was already opened, it is silently closed before being reused.
 *
 *  Args:
 *      oldfd: The file descriptor to copy.
 *      newfd: The new file descriptor to use.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      On success, the new file descriptor is returned.  On error, SKID_BAD_FD is returned
 *      (check errnum for details).
 */
int call_dup2(int oldfd, int newfd, int *errnum);

/*
 *  Description:
 *      Open a file descriptor using open().
 *
 *  Args:
 *      filename: A filename, relative or absolute, to open.
 *      flags: The flags to pass to open() (see: open(2)).
 *      mode: The mode to pass to open() (see: open(2)).
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      On success, the opened file descriptor is returned.  On error, SKID_BAD_FD is returned
 *      (check errnum for details).
 */
int open_fd(const char *filename, int flags, mode_t mode, int *errnum);

/*
 *  Description:
 *      Read the contents of the file descriptor into a heap-allocated buffer.  It is the caller's
 *      responsibility to free the buffer with free_skid_mem().
 *
 *  Args:
 *      fd: File descriptor to read from.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Pointer to the heap-allocated buffer, on success.  NULL on error (check errnum for details).
 */
char *read_fd(int fd, int *errnum);

/*
 *    Description:
 *        Write a string to the file descriptor.
 *
 *    Args:
 *        fd: File descriptor to write to.
 *        msg: The nul-terminated message to write to fd.
 *
 *    Returns:
 *        On success, zero is returned.  On error, errno is returned.
 */
int write_fd(int fd, const char *msg);

#endif  /* __SKID_FILE_DESCRIPTORS__ */
