/*
 *	This library contains non-releasable, unit-test-specific, miscellaneous helper code.
 */

#ifndef __SKIP_DEVOPS__
#define __SKIP_DEVOPS__

#include <stdbool.h>    // bool, false, true
#include <stddef.h>		// size_t

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
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int free_devops_mem(char **old_array);

/*
 *  Description:
 *      Get the actual file permissions for pathname by executing the following command in a shell:
 *          stat -c %b <pathname>
 *      This is intended as a double-do to validate the results of skip_file_metadata_read's
 *      get_block_count() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch permissions for.
 *      errnum: [Out] Storage location for errno values encountered.  Set to zero on success.
 *
 *  Returns:
 *      Block count on success, which is legitimately 0 in some cases.
 *      The only way to detect errors in execution is to check errnum an errno value (non-zero).
 */
blkcnt_t get_shell_block_count(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the pathnames device ID by executing the following command in a shell:
 *          stat -c %d <pathname>
 *      This is intended as a double-do to validate the results of skip_file_metadata_read's
 *      get_file_device_id() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch permissions for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      File permissions on success, 0 on error.  Check errnum for actual errno value.
 */
dev_t get_shell_device_id(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the actual file permissions for pathname by executing the following command in a shell:
 *          stat -c %a <pathname>
 *      This is intended as a double-do to validate the results of skip_file_metadata_read's
 *      get_file_perms() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch permissions for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      File permissions on success, 0 on error.  Check errnum for actual errno value.
 */
mode_t get_shell_file_perms(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the ID of pathname's group by executing the following command in a shell:
 *          stat -c %g <pathname>
 *      This is intended as a double-do to validate the results of skip_file_metadata_read's
 *      get_group() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the group ID for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Group ID on success.  If there's an error, errnum will be set with the actual errno value.
 *      On success, errnum will be zeroized.
 */
gid_t get_shell_group(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the ID of pathname's owner by executing the following command in a shell:
 *          stat -c %u <pathname>
 *      This is intended as a double-do to validate the results of skip_file_metadata_read's
 *      get_owner() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the owner ID for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Owner ID on success.  If there's an error, errnum will be set with the actual errno value.
 *      On success, errnum will be zeroized.
 */
uid_t get_shell_owner(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the actual block size of the filesystem mounted in current directory by executing:
 *          stat -fc %s .
 *      This is intended as a double-do to validate the results of skip_file_metadata_read's
 *      get_block_size() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Filesystem block size on success, -1 on error.  Check errnum for actual errno value.
 */
long get_sys_block_size(int *errnum);

/*
 *  Description:
 *      Use mknod() to create a named pipe.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, for the named pipe.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int make_a_pipe(const char *pathname);

/*
 *  Description:
 *      Use socket() to create a raw socket for local communication and bind it to filename.
 *      When no longer required, the socket filename should be deleted using unlink(2) or remove(3).
 *      It is the caller's responsibility to delete the socket filename by manually deleting it
 *      or calling remove_a_file() (since that function utilizes remove()).
 *
 *  Args:
 *      filename: The filename, relative or absolute, for the raw local socket.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int make_a_socket(const char *filename);

/*
 *  Description:
 *      Use remove() to delete a file.
 *
 *  Args:
 *      filename: The filename, relative or absolute, to delete.
 *      ignore_missing: Treat a missing filename as success.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int remove_a_file(const char *filename, bool ignore_missing);

/*
 *  Description:
 *      Translate rel_filename into an absolute filename resolved to the repo_name, as extracted
 *		from the current working directory.  Caller is responsible for calling devops_free().
 *
 *  Args:
 *      repo_name: Root-level directory name to find in the current working directory
 *			(e.g., SKIP_REPO_NAME).
 *      rel_filename: Optional; The relative filename to resolve to repo_name's absolute path.
 *			If this argument is NULL, only the repo_dir will be returned.
 *		must_exist: If true, the resulting repo_dir/rel_filename must exist or errnum will
 *			be updated with ENOENT.  If false, it doesn't matter (e.g., "missing file" error input).
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated array representing repo_dir/rel_filename on success.  Caller is
 *		respsonsible for freeing the return value with free_devops_mem().
 *		NULL on error (check errnum).
 */
char *resolve_to_repo(const char *repo_name, const char *rel_filename, bool must_exist,
                      int *errnum);

/*
 *  Description:
 *      Uses popen to execute command in a read-only process and read the results into output.
 *
 *  Args:
 *      command: The command to execute.
 *      output: Optional; [Out] The output from command will be read into this buffer, if a valid
 *          pointer.
 *      output_len: Optional; If output is to be used, this value indicates the size of output.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int run_command(const char *command, char *output, size_t output_len);

/*
 *  Description:
 *      Uses popen to execute command + pathname in a read-only process and read the results
 *      into output.  If there needs to be a space between command and pathname, ensure
 *      command has a trailing space.
 *
 *  Args:
 *      command: The preface command to execute.
 *      pathname: The pathname, relative or absolute, to append to command prior to execution.
 *      output: Optional; [Out] The output from command will be read into this buffer, if a valid
 *          pointer.
 *      output_len: Optional; If output is to be used, this value indicates the size of output.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int run_path_command(const char *command, const char *pathname, char *output, size_t output_len);

#endif  /* __SKIP_DEVOPS__ */
