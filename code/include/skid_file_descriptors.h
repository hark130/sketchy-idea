#ifndef __SKID_FILE_DESCRIPTORS__
#define __SKID_FILE_DESCRIPTORS__

#include <stdbool.h>    	// bool, false, true
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
 *		Read the contents of filename into a heap-allocated buffer.  It is the caller's
 *		responsibility to free the buffer with free_skid_mem().
 *
 *	Args:
 *		fd: File descriptor to read from.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Pointer, on success.  NULL on error (check errnum for details).
 */
char *read_fd(int fd, int *errnum);

#endif  /* __SKID_FILE_DESCRIPTORS__ */
