#ifndef __SKID_FILE_METADATA_WRITE__
#define __SKID_FILE_METADATA_WRITE__

#include <errno.h>            // errno
#include <fcntl.h>            // S_I* mode macros
#include <stdbool.h>        // bool, false, true
#include <sys/types.h>        // time_t
#include "skid_macros.h"    // SKID_MODE_* macros

/*
 *  Description:
 *        Adds to pathname's permission bits by fetching the current mode, ORing more_mode,
 *        and then calling set_mode() with the new mode.  Symbolic links are always dereferenced.
 *        The additional permissions for pathname are specified in more_mode, which is
 *        a bit mask created by ORing together zero or more SKID_MODE_* macros (or S_I* macros from
 *        chmod(2)).
 *
 *        See set_mode() for important notes.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *        more_mode: The additional permission flags to add to pathname's mode.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int add_mode(const char *pathname, mode_t more_mode);

/*
 *  Description:
 *        Removes the less_mode permission bits from pathname by fetching the current mode,
 *        removing less_mode bits, and then calling set_mode() with the new mode.
 *        Symbolic links are always dereferenced.  The additional permissions for pathname are
 *        specified in less_mode, which is a bit mask created by ORing together zero or more
 *        SKID_MODE_* macros (or S_I* macros from chmod(2)).
 *
 *        See set_mode() for important notes.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *        less_mode: The permission flags to remove from pathname's mode.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int remove_mode(const char *pathname, mode_t less_mode);

/*
 *  Description:
 *        Changes the file metadata of pathname's access time to the provided values using
 *        utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the atime of.  If pathname is relative,
 *            pathname will be resolved against the current working directory.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *        seconds: The epoch seconds to set the atime to.
 *        nseconds: The nanoseconds to set the atime to.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_atime(const char *pathname, bool follow_sym, time_t seconds, long nseconds);

/*
 *  Description:
 *        Changes the file metadata of pathname's access time to the current local time using
 *        utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the atime of.  If pathname is relative,
 *            pathname will be resolved against the current working directory.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_atime_now(const char *pathname, bool follow_sym);

/*
 *  Description:
 *        Changes pathname's owner by calling chown(), or lchown() if follow_sym is false.
 *
 *        IMPORTANT NOTES:
 *            - Only a privileged process (Linux: one with the CAP_CHOWN capability) may change the
 *                 owner of a file.
 *            - The owner of a file may change the group of the file to any group
 *              of which that owner is a member.
 *            - A privileged process (Linux: with CAP_CHOWN) may change the group arbitrarily.
 *            - When the owner or group of an executable file is changed by an unprivileged user,
 *              the S_ISUID and S_ISGID mode bits are cleared.  Specific kernel implementations
 *              control whether or not this also happens for privileged users.
 *            - See chown(2) for details.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *        new_group: The GID of the new group for pathname.  If this value is -1, it will not change.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_group_id(const char *pathname, gid_t new_group, bool follow_sym);

/*
 *  Description:
 *        Changes pathname's permission bits to new_mode by calling chmod().  Symbolic links are
 *        always dereferenced.  The new file mode for pathname is specified in new_mode, which is
 *        a bit mask created by ORing together zero or more SKID_MODE_* macros (or S_I* macros from
 *        chmod(2)).
 *
 *        IMPORTANT NOTES:
 *            - The effective UID of the calling process must match the owner of the file, or the
 *              process must be privileged.
 *            - If the calling process is not privileged, and the group of the file does not
 *              match the effective group ID of the process or one of its supplementary group IDs,
 *              the set-group-ID bit will be turned off, but this will not cause an error to be
 *              returned.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *        new_mode: The new mode for pathname.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_mode(const char *pathname, mode_t new_mode);

/*
 *  Description:
 *        Changes the file metadata of pathname's modification time to the provided values using
 *        utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the mtime of.  If pathname is relative,
 *            pathname will be resolved against the current working directory.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *        seconds: The epoch seconds to set the mtime to.
 *        nseconds: The nanoseconds to set the mtime to.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_mtime(const char *pathname, bool follow_sym, time_t seconds, long nseconds);

/*
 *  Description:
 *        Changes the file metadata of pathname's modification time to the current local time using
 *        utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the mtime of.  If pathname is relative,
 *            pathname will be resolved against the current working directory.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_mtime_now(const char *pathname, bool follow_sym);

/*
 *  Description:
 *        Changes pathname's owner by calling chown(), or lchown() if follow_sym is false.
 *
 *        IMPORTANT NOTES:
 *            - Only a privileged process (Linux: one with the CAP_CHOWN capability) may change the
 *                 owner of a file.
 *            - When the owner or group of an executable file is changed by an unprivileged user,
 *              the S_ISUID and S_ISGID mode bits are cleared.  Specific kernel implementations
 *              control whether or not this also happens for privileged users.
 *            - See chown(2) for details.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *        new_owner: The UID of the new owner for pathname.  If this value is -1, it will not change.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_owner_id(const char *pathname, uid_t new_owner, bool follow_sym);

/*
 *  Description:
 *        Changes pathname's owner and group by calling chown(), or lchown() if follow_sym is false.
 *
 *        IMPORTANT NOTES:
 *            - Only a privileged process (Linux: one with the CAP_CHOWN capability) may change the
 *                 owner of a file.
 *            - The owner of a file may change the group of the file to any group
 *              of which that owner is a member.
 *            - A privileged process (Linux: with CAP_CHOWN) may change the group arbitrarily.
 *            - When the owner or group of an executable file is changed by an unprivileged user,
 *              the S_ISUID and S_ISGID mode bits are cleared.  Specific kernel implementations
 *              control whether or not this also happens for privileged users.
 *            - See chown(2) for details.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the group of.
 *        new_owner: The UID of the new owner for pathname.  If this value is -1, it will not change.
 *        new_group: The GID of the new group for pathname.  If this value is -1, it will not change.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_ownership(const char *pathname, uid_t new_owner, gid_t new_group, bool follow_sym);

/*
 *  Description:
 *        Changes the file metadata of pathname's access and modification times to both match
 *        the provided values using utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the mtime of.  If pathname is relative,
 *            pathname will be resolved against the current working directory.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *        seconds: The epoch seconds to set the mtime to.
 *        nseconds: The nanoseconds to set the mtime to.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_times(const char *pathname, bool follow_sym, time_t seconds, long nseconds);

/*
 *  Description:
 *        Changes the file metadata of pathname's access and modification times to the current local
 *        time using utimensat().
 *
 *  Args:
 *      pathname: Absolute or relative pathname to modify the atime and mtime of.  If pathname
 *            is relative, pathname will be resolved against the current working directory.
 *        follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *            do not follow symbolic links if false.
 *
 *  Returns:
 *        0, on success.  On failure, an errno value.
 */
int set_times_now(const char *pathname, bool follow_sym);

#endif  /* __SKID_FILE_METADATA_WRITE__ */
