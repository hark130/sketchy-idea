/*
 *	This library contains non-releasable, unit-test-specific, miscellaneous helper code.
 */

#ifndef __SKID_DEVOPS__
#define __SKID_DEVOPS__

#include <stdbool.h>    // bool, false, true
#include <stddef.h>     // size_t
#include <sys/types.h>	// blkcnt_t
#include <time.h>		// time_t
#include <unistd.h>     // useconds_t

// Baseline dir level to standardize file-based test input paths
#define SKID_REPO_NAME (const char *)"sketchy-idea"  // The name of this repo
#define SKID_MAX_FILES 100  						 // Max files create_path_tree() will create
#define SKID_MAX_DEPTH 5	  						 // Max depth create_path_tree() will create

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
void *alloc_devops_mem(size_t num_elem, size_t size_elem, int *errnum);

/*
 *  Description:
 *      Queue a signal, with integer data, to a PID by calling sigqueue() (see: sigqueue(3)).
 *
 *  Args:
 *      pid: The PID to queue the signal for.
 *		signum: The signal number to queue.
 *		sival_int: The integer value to include with the signal.
 *
 *	Returns:
 *		0 on success, errno on error.
 */
int call_sigqueue(pid_t pid, int signum, int sival_int);

/*
 *	Description:
 *		Allocate a character array in heap memory and copy source in.
 *		Caller is respsonsible for freeing the return value with free_devops_mem().
 *
 *	Args:
 *		source: The string to copy.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated character array containing a copy of source on success.
 *		NULL on error (check errnum).
 */
char *copy_string(const char *source, int *errnum);

/*
 *	Description:
 *		Recursively create top_dir with a file/dir hierarchy underneath it.
 *		Each directory will have num_files number of files inside.  There will be
 *		tree_depth number of sub-directories found within top_dir.  Each level
 *		will have tree_width number of directories within it.  Directory names and
 *		filenames will be basic (e.g., dir# and file# respectively).  Files will be
 *		created with their filename, using top_dir as the root, in an attempt to
 *		resonably create unique files.  This function will not create more than
 *		SKID_MAX_FILES number of files.
 *
 *	Notes:
 *		It is the caller's responsibility to free the memory returned by this function.
 *		Each string pointer must be individually freed.  The array pointer must also be freed.
 *		Use free_devops_mem() to free individual string pointers (AKA pick-and-choose) or
 *		use free_path_tree() to free the entire collection.
 *
 *	Args:
 *		top_dir: Top level directory, relative or absolute, to create.  Must not exist.
 *		num_files: Number of files to create at each directory level.  Values of zero
 *			will result in an empty directory hierarchy.
 *		tree_width: Number of directories to create at each level.  Values of zero will
 *			result in top_dir being devoid of directories.
 *		tree_depth: Number of sub-directory levels to create within top_dir.  Values of zero
 *			will result in top_dir being devoid of directories.  This value controls the
 *			recursive execution of this function.  This function will not support tree_depth
 *			values that exceed SKID_MAX_DEPTH.
 *		errnum: [Out] Storage location for error values.  0 on success.  Errno value, or -1 for
 *			unspecified errors, on failure.  EMFILE is used if the requested number of files
 *			is larger than SKID_MAX_FILES.
 *
 *	Returns:
 *		A NULL-terminated array of string pointers on success.  NULL on failure (see: errnum).
 */
char **create_path_tree(const char *top_dir, unsigned int num_files, unsigned int tree_width,
	                    unsigned int tree_depth, int *errnum);

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
int free_devops_mem(void **old_array);

/*	Description:
 *		Free the return value provided by create_path_tree() and set the original pointer to NULL.
 *
 *	Args:
 *		old_path_tree: Pointer to the return value returned by create_path_tree().
 *
 *	Returns:
 *		0 on success, errno on error.
 */
int free_path_tree(char ***old_path_tree);

/*
 *  Description:
 *      Determine the parent directory of path.
 *		Caller is respsonsible for freeing the return value with free_devops_mem().
 *
 *  Args:
 *      path: A non-empty path that may or may not exist.
 *
 *  Returns:
 *		Heap-allocated character array containing path's parent directory on success.
 *		NULL on error.
 */
char *get_parent_dir(const char *path);

/*
 *  Description:
 *      Get the pathname's raw access time by executing the following command in a shell:
 *          stat -c %X <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_access_time() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the atime for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Raw access time on success, 0 on error.  Check errnum for actual errno value.
 */
time_t get_shell_atime(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the actual file block count for pathname by executing the following command in a shell:
 *          stat -c %b <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_block_count() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the block count for.
 *      errnum: [Out] Storage location for errno values encountered.  Set to zero on success.
 *
 *  Returns:
 *      Block count on success, which is legitimately 0 in some cases.
 *      The only way to detect errors in execution is to check errnum an errno value (non-zero).
 */
blkcnt_t get_shell_block_count(const char *pathname, int *errnum);

/*
 *  Description:
 *      Using devops_code run_command-family functions:
 *          1. Get the current user's username (using get_shell_my_username())
 *          2. Parse /etc/group for all GIDs "my username" is a member of
 *          3. Store those GIDs in a heap-allocated array of gid_t values
 *          4. Terminate the array with "my username"'s GID (using get_shell_*() functionality)
 *      This is intended as a double-do to facilitate testing of skid_file_metadata_write's
 *      set_ownership() without having to hard-code brittle expected return values.
 *      The caller is responsible for using free_devops_mem() to free the array of gid_t's
 *      returned by this function.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      A "my GID"-terminated heap-allocated array of compatible GIDs on success.
 *      NULL on error.  Check errnum for actual errno value.
 */
gid_t *get_shell_compatible_gid(int *errnum);

/*
 *  Description:
 *      Get the pathname's raw status change time by executing the following command in a shell:
 *          stat -c %Z <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_change_time() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the ctime for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Raw status change time on success, 0 on error.  Check errnum for actual errno value.
 */
time_t get_shell_ctime(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the pathnames device ID by executing the following command in a shell:
 *          stat -c %d <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_file_device_id() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the device ID for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      File device ID on success, 0 on error.  Check errnum for actual errno value.
 */
dev_t get_shell_device_id(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the actual file permissions for pathname by executing the following command in a shell:
 *          stat -c %a <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
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
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_group() without having to hard-code brittle expected return values.
 *		This usage of the stat command does *not* follow symbolic links.
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
 *      Get the number of hard links to pathname by executing the following command in a shell:
 *          stat -c %h <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_get_hard_link_num() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to get the number of hard links for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Number of hard links to pathname on success.  If there's an error, errnum will be set
 *      with the actual errno value.  On success, errnum will be zeroized.
 */
nlink_t get_shell_hard_links(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get pathname's inode number by executing the following command in a shell:
 *          stat -c %i <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_serial_number() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the inode number for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Inode number on success.  If there's an error, errnum will be set with the actual errno
 *      value.  On success, errnum will be zeroized.
 */
ino_t get_shell_inode(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the pathname's raw modification time by executing the following command in a shell:
 *          stat -c %Y <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_mod_time() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the mtime for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Raw modification time on success, 0 on error.  Check errnum for actual errno value.
 */
time_t get_shell_mtime(const char *pathname, int *errnum);

/*
 *  Description:
 *      Get the current user's GID by executing the following command in a shell:
 *          id -g
 *      This is intended as a double-do to facilitate testing of skid_file_metadata_write's
 *      set_ownership() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      GID on success, 0 on error.  The only true indication of an error is the errnum value
 *      since 0 is a valid GID.
 */
gid_t get_shell_my_gid(int *errnum);

/*
 *  Description:
 *      Get the current user's UID by executing the following command in a shell:
 *          id -u
 *      This is intended as a double-do to facilitate testing of skid_file_metadata_write's
 *      set_ownership() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      UID on success, 0 on error.  The only true indication of an error is the errnum value
 *      since 0 is a valid UID.
 */
uid_t get_shell_my_uid(int *errnum);

/*
 *  Description:
 *      Get the current user's username by executing the following command in a shell:
 *          whoami
 *      This is intended as a double-do and not meant to be "production code".
 *      Also, the caller is responsible for using free_devops_mem() to free the memory address
 *      returned by this function.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated string containing the username on success, NULL on error.
 *      Check errnum for actual errno value on error.
 */
char *get_shell_my_username(int *errnum);

/*
 *	Description:
 *		Get the current nanoseconds by executing the following command in a shell:
 *      	date '+%N'
 *		This is intended to help create unique directory names to unit test the dir_ops library.
 *
 *	Args:
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Raw time on success, 0 on error.  Check errnum for actual errno value.
 */
long get_shell_nsec_now(int *errnum);

/*
 *  Description:
 *      Get the ID of pathname's owner by executing the following command in a shell:
 *          stat -c %u <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
 *      get_owner() without having to hard-code brittle expected return values.
 *		This usage of the stat command does *not* follow symbolic links.
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
 *      Get the size, in bytes, of pathname by executing the following command in a shell:
 *          stat -c %s <pathname>
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
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
off_t get_shell_size(const char *pathname, int *errnum);

/*
 *	Description:
 *		Get the current time, in seconds from epoch, by executing the following command in a shell:
 *			date '+%s'
 *		This is intended to help create unique directory names to unit test the dir_ops library.
 *
 *	Args:
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Raw time on success, 0 on error.  Check errnum for actual errno value.
 */
time_t get_shell_time_now(int *errnum);

/*
 *	Description:
 *		Get the current user's umask by executing the following command in a shell:
 *			umask
 *		This is intended to help programmatically determine expected results of directory
 *		permissions created by create_dir().
 *
 *	Args:
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		The umask value on success, 0 on error.  Check errnum for actual errno value.
 */
mode_t get_shell_umask(int *errnum);

/*
 *  Description:
 *      Get the username's GID by executing the following command in a shell:
 *          id -g <username>
 *      This is intended as a double-do to facilitate testing of skid_file_metadata_write's
 *      set_ownership() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      username: The name to fetch the GID for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      GID on success, 0 on error.  The only true indication of an error is the errnum value
 *      since 0 is a valid GID.
 */
gid_t get_shell_user_gid(const char *username, int *errnum);

/*
 *  Description:
 *      Get the username's UID by executing the following command in a shell:
 *          id -u <username>
 *      This is intended as a double-do to facilitate testing of skid_file_metadata_write's
 *      set_ownership() without having to hard-code brittle expected return values.
 *
 *  Args:
 *      username: The name to fetch the UID for.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      UID on success, 0 on error.  The only true indication of an error is the errnum value
 *      since 0 is a valid UID.
 */
uid_t get_shell_user_uid(const char *username, int *errnum);

/*
 *  Description:
 *      Get the actual block size of the filesystem mounted in current directory by executing:
 *          stat -fc %s .
 *      This is intended as a double-do to validate the results of skid_file_metadata_read's
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
 *	Description:
 *		Answers the question, "Does pathname exist?".  Any invalid input is treated as a "no".
 *		The following errno values are also treated as a "no":
 *			ENOENT is obvious... file flat out doesn't exist.
 *			ENAMETOOLONG means pathname is too long to it *can't* exist.
 *			ENOTDIR means part of the path prefix of pathname is not a dir so it *can't* exist.
 *		EACCES is tricky because it could be inconclusive.  Permission could be denied for one
 *		of the directories in the path prefix of pathname.  Still, this function treats EACCES
 *		as a "yes" because this is devops code (but prints a DEBUG warning).
 *
 *	Args:
 *		pathname: Absolute or relative pathname to check.
 *
 *	Returns:
 *		True if pathname exists.  False otherwise.
 */
bool is_path_there(const char *pathname);

/*
 *	Description:
 *		Allocate heap-memory and concatenate dirname/pathname, adding a delimiter if necessary.
 *		The caller is responsible for using free_devops_mem() to free the memory address returned
 *		by this function.
 *
 *	Args:
 *		dirname: The absolute or relative directory name which contains pathname.
 *		pathname: Optional; Absolute or relative pathname to join to dirname.  This function
 *			will add a delimiter between dirname/pathname if pathname is defined.  Ignored if NULL
 *			or empty.
 *		must_exist: If true, dirname must exist.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated memory address containing dirname/pathname, on success.
 *		Returns NULL on error.  Check errnum for errno value.
 */
char *join_dir_to_path(const char *dirname, const char *pathname, bool must_exist, int *errnum);

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
 *      Call usleep() to sleep for a number of microseconds.
 *
 *  Args:
 *      num_microsecs: The number of microseconds to sleep.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int micro_sleep(useconds_t num_microsecs);

/*
 *  Description:
 *      Read the contents of filename into a heap-allocated buffer.  The caller is responsible
 *      for using free_devops_mem() to free the memory address returned by this function.
 *
 *  Args:
 *      filename: The relative or absolute path of a filename to read.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Pointer to a heap-allocated buffer, which contains the contents of filename, on success.
 *      Returns NULL on error.  Check errnum for errno value (or -1 on unspecified error).
 */
char *read_a_file(const char *filename, int *errnum);

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
 *	Description:
 *		Remove and empty directory by executing the following command in a shell:
 *			rmdir <dirname>
 *		This is intended as a double-do to facilitate testing of skid_dir_operations's
 *		create_dir() by cleaning up temp directories created at "test time".
 *
 *	Args:
 *		dirname: The name, relative or absolute, of an empty directory to remove.
 *
 *	Returns:
 *		0 on success.  On error, errno value or -1 for an unspecified error.
 */
int remove_shell_dir(const char *dirname);

/*
 *  Description:
 *      Translate rel_filename into an absolute filename resolved to the repo_name, as extracted
 *		from the current working directory.  Caller is responsible for calling devops_free().
 *
 *  Args:
 *      repo_name: Root-level directory name to find in the current working directory
 *			(e.g., SKID_REPO_NAME).
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
 *      Uses popen to execute base_cmd + cmd_suffix in a read-only process and read the results
 *      into output.  If there needs to be a space between base_cmd and cmd_suffix, ensure
 *      base_cmd has a trailing space.
 *
 *  Args:
 *      base_cmd: The preface command to execute.
 *      cmd_suffix: Optional; The suffix to add to base_cmd prior to execution.  If NULL or
 *          empty, just the base_cmd will be executed.
 *      output: Optional; [Out] The output from command will be read into this buffer, if a valid
 *          pointer.
 *      output_len: Optional; If output is to be used, this value indicates the size of output.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int run_command_append(const char *base_cmd, const char *cmd_suffix, char *output,
					   size_t output_len);

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

/*
 *  Description:
 *      Set pathname's permissions by executing the following command in a shell:
 *          chmod <new_perms> <pathname>
 *      This is intended as a double-do to facilitate testing of skid_file_metadata_write's
 *      set_mode()-family of functions.
 *
 *  Args:
 *      pathname: The pathname, absolute or relative, to set the permissions for.
 *      new_perms: The new permissions, as an octal number, to set for pathname.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int set_shell_perms(const char *pathname, mode_t new_perms);

#endif  /* __SKID_DEVOPS__ */
