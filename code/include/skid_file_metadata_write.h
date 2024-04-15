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
 *		Changes pathname's owner by calling chown(), or lchown() if follow_sym is false.
 *
 *		IMPORTANT NOTES:
 *			- Only a privileged process (Linux: one with the CAP_CHOWN capability) may change the
 *	   		  owner of a file.
 *			- The owner of a file may change the group of the file to any group
 *			  of which that owner is a member.
 *			- A privileged process (Linux: with CAP_CHOWN) may change the group arbitrarily.
 *			- When the owner or group of an executable file is changed by an unprivileged user,
 *			  the S_ISUID and S_ISGID mode bits are cleared.  Specific kernel implementations
 *			  control whether or not this also happens for privileged users.
 *			- See chown(2) for details.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *		new_group: The GID of the new group for pathname.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_group_id(const char *pathname, gid_t new_group, bool follow_sym);

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
 *		Changes pathname's owner by calling chown(), or lchown() if follow_sym is false.
 *
 *		IMPORTANT NOTES:
 *			- Only a privileged process (Linux: one with the CAP_CHOWN capability) may change the
 *	   		  owner of a file.
 *			- When the owner or group of an executable file is changed by an unprivileged user,
 *			  the S_ISUID and S_ISGID mode bits are cleared.  Specific kernel implementations
 *			  control whether or not this also happens for privileged users.
 *			- See chown(2) for details.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *		new_owner: The UID of the new owner for pathname.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_owner_id(const char *pathname, uid_t new_owner, bool follow_sym);

/*
 *  Description:
 *		Changes pathname's owner and group by calling chown(), or lchown() if follow_sym is false.
 *
 *		IMPORTANT NOTES:
 *			- Only a privileged process (Linux: one with the CAP_CHOWN capability) may change the
 *	   		  owner of a file.
 *			- The owner of a file may change the group of the file to any group
 *			  of which that owner is a member.
 *			- A privileged process (Linux: with CAP_CHOWN) may change the group arbitrarily.
 *			- When the owner or group of an executable file is changed by an unprivileged user,
 *			  the S_ISUID and S_ISGID mode bits are cleared.  Specific kernel implementations
 *			  control whether or not this also happens for privileged users.
 *			- See chown(2) for details.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *		new_owner: The UID of the new owner for pathname.
 *		new_group: The GID of the new group for pathname.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *
 *  Returns:
 *		0, on success.  On failure, an errno value.
 */
int set_ownership(const char *pathname, uid_t new_owner, gid_t new_group, bool follow_sym);

/*
 *  Description:
 *		Changes the file metadata of pathname's access and modification times to both match
 *		the provided values using utimensat().
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
int set_times(const char *pathname, bool follow_sym, time_t seconds, long nseconds);

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
