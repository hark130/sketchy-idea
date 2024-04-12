#ifndef __SKID_FILE_METADATA_WRITE__
#define __SKID_FILE_METADATA_WRITE__

#include <errno.h>		// errno
#include <stdbool.h>    // bool, false, true
#include <sys/types.h>	// time_t

/*
 *  Description:
 *		Changes the file metadata of pathname's access time to the provided values using
 *		utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the atime of.  If pathname is relative,
 *			pathname will be resolved against the current working directory.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *		seconds: The epoch seconds to set the atime to.
 *		nseconds: The nanoseconds to set the atime to.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_atime(const char *pathname, bool follow_sym, time_t seconds, long nseconds);

/*
 *  Description:
 *		Changes the file metadata of pathname's access time to the current local time using
 *		utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the atime of.  If pathname is relative,
 *			pathname will be resolved against the current working directory.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_atime_now(const char *pathname, bool follow_sym);

/*
 *  Description:
 *		Changes the file metadata of pathname's modification time to the provided values using
 *		utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the mtime of.  If pathname is relative,
 *			pathname will be resolved against the current working directory.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *		seconds: The epoch seconds to set the mtime to.
 *		nseconds: The nanoseconds to set the mtime to.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_mtime(const char *pathname, bool follow_sym, time_t seconds, long nseconds);

/*
 *  Description:
 *		Changes the file metadata of pathname's modification time to the current local time using
 *		utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the mtime of.  If pathname is relative,
 *			pathname will be resolved against the current working directory.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_mtime_now(const char *pathname, bool follow_sym);

/*
 *  Description:
 *		Changes the file metadata of pathname's access and modification times to the current local
 *		time using utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the atime and mtime of.  If pathname
 *			is relative, pathname will be resolved against the current working directory.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_times_now(const char *pathname, bool follow_sym);

#endif  /* __SKID_FILE_METADATA_WRITE__ */
