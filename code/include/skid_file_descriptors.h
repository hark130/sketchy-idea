#ifndef __SKID_FILE_DESCRIPTORS__
#define __SKID_FILE_DESCRIPTORS__

#include <stdbool.h>    	// bool, false, true
#include <stddef.h>			// size_t
#include "skid_macros.h"	// SKID_BAD_FD

/*
 *	Description:
 *		Close a file descriptor and sets it to SKID_BAD_FD (if it was successfully closed).
 *
 *	Args:
 *		fdp: [In/Out] A pointer to a file descriptor to close.
 *		quiet: If true, silences all logging/debugging.
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int close_fd(int *fdp, bool quiet);

/*
 *	Description:
 *		Read the contents of the file descriptor into a heap-allocated buffer.  It is the caller's
 *		responsibility to free the buffer with free_skid_mem().
 *
 *	Args:
 *		fd: File descriptor to read from.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Pointer to the heap-allocated buffer, on success.  NULL on error (check errnum for details).
 */
char *read_fd(int fd, int *errnum);

/*
 *	Description:
 *		Write a string to the file descriptor.
 *
 *	Args:
 *		fd: File descriptor to write to.
 *		msg: The nul-terminated message to write to fd.
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int write_fd(int fd, const char *msg);

#endif  /* __SKID_FILE_DESCRIPTORS__ */
