#ifndef __SKID_DIR_OPERATIONS__
#define __SKID_DIR_OPERATIONS__

#include <stdbool.h>    // bool, false, true
#include <sys/stat.h>	// mode_t

/*
 *	Description:
 *		Create a new directory, dirname, with permissions, mode, by calling mkdir().
 *
 *	Notes:
 *		The argument mode is modified by the process's umask in the usual way: normally, the mode
 *		of the created directory is (mode & ~umask & 0777).  Whether other mode bits are
 *		honored for the created directory depends on the operating system.
 *
 *		For Linux, the newly created directory will be owned by the effective user ID of
 *		the process.  If the directory containing the file has the set-group-ID bit set,
 *		or if the filesystem is mounted with BSD group semantics the new directory will
 *		inherit the group ownership from its parent; otherwise it will be owned by the effective
 *		group ID of the process.  If the parent directory has the set-group-ID bit set, then
 *		so will the newly created directory.
 *
 *	Args:
 *		dirname: Absolute or relative directory to create.
 *		mode: Specifies the mode for the new directory.
 *
 *	Returns:
 *		0, on success.  On failure, an errno value.
 */
int create_dir(const char *dirname, mode_t mode);

/*
 *	Description:
 *		Remove a directory, dirname, by calling rmdir().  The dirname directory must be empty.
 *
 *	Notes:
 *		TO DO: DON'T DO NOW...
 *
 *	Args:
 *		dirname: Absolute or relative directory to delete.
 *
 *	Returns:
 *		0, on success.  On failure, an errno value.
 */
int delete_dir(const char *dirname);

/*
 *	Description:
 *		Remove a directory, dirname, by recursively deleting all files and directories contained
 *		within.
 *
 *	Notes:
 *		TO DO: DON'T DO NOW...
 *
 *	Args:
 *		dirname: Absolute or relative directory to destroy.
 *
 *	Returns:
 *		0, on success.  On failure, an errno value.
 */
int destroy_dir(const char *dirname);

/*
 *	Description:
 *		Read the contents of dirname into a heap-allocated, NULL-terminated, array of string
 *		buffers.
 *
 *	Notes:
 *		TO DO: DON'T DO NOW...
 *
 *	Args:
 *		dirname: Absolute or relative directory to create.
 *		recurse: If true, also include all sub-dirs and their files in the array.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		0, on success.  On failure, an errno value.
 */
char **read_dir_contents(const char *dirname, bool recurse, int *errnum);

#endif  /* __SKID_DIR_OPERATIONS__ */
