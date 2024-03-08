/*
 *	This library contains non-releasable, unit-test-specific, miscellaneous helper code.
 */

#ifndef __SKIP_DEVOPS__
#define __SKIP_DEVOPS__

#include <stdbool.h>    // bool, false, true

// Baseline dir level to standardize file-based test input paths
#define SKIP_REPO_NAME (const char *)"sketchy-idea"  // The name of this repo

/*
 *  Description:
 *      Allocate zeroized array in heap memory.
 *
 *  Args:
 *      num_elem: The number of elements in the array.
 *		size_elem: The size of each element in the array.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated array of total size num_elem * size_elem that has been zeroized, on success.
 *		Caller is respsonsible for freeing the return value with free_devops_mem().
 *		NULL on error (check errnum).
 */
char *alloc_devops_mem(size_t num_elem, size_t size_elem, int *errnum);

/*
 *  Description:
 *      Free a devops-allocated array from heap memory and set the original pointer to NULL.
 *
 *  Args:
 *      old_array: Pointer to the heap-allocated array to free storage location.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int free_devops_mem(char **old_array);

/*
 *  Description:
 *      Translate rel_filename into an absolute filename resolved to the root_dir, as extracted
 *		from the current working directory.  Caller is responsible for calling devops_free().
 *
 *  Args:
 *      root_dir: Root-level directory name to find in the current working directory.
 *      rel_filename: The relative filename to resolve to the root_dir.
 *		must_exist: If true, the resulting root_dir/rel_filename must exist or errnum will
 *			be updated with ENOENT.  If false, it doesn't matter (e.g., "missing file" error input).
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated array representing root_dir/rel_filename on success.  Caller is
 *		respsonsible for freeing the return value with free_devops_mem().
 *		NULL on error (check errnum).
 */
char *get_repo_path(const char *root_dir, const char *rel_filename, bool must_exist, int *errnum);

#endif  /* __SKIP_DEVOPS__ */
