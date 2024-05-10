#ifndef __SKID_DIR_OPERATIONS__
#define __SKID_DIR_OPERATIONS__

#include <stdbool.h>    // bool, false, true
#include <sys/stat.h>	// mode_t

/*
 *	Description:
 *		Create a new directory, dirname, with permissions, mode, by calling mkdir().
 *
 *	Notes:
 *		TO DO: DON'T DO NOW...
 *
 *	Args:
 *		dirname: Absolute or relative directory to create.
 *		mode: The permission bits for the new directory.
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
