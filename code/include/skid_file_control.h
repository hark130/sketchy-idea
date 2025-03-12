#ifndef __SKID_FILE_CONTROL__
#define __SKID_FILE_CONTROL__

#include <stdbool.h>    // bool, false, true

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

#endif  /* __SKID_FILE_CONTROL__ */
