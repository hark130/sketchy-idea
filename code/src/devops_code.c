/*
 *  This library contains non-releasable, unit-test-specific, miscellaneous helper code.
 */

#define SKID_DEBUG			// Enable DEBUG logging

#include <errno.h>          // errno
#include <limits.h>         // PATH_MAX
#include <stdio.h>          // remove()
#include <stdlib.h>         // calloc(), free()
#include <string.h>         // strstr()
#include <sys/socket.h>     // AF_UNIX, socket()
#include <sys/stat.h>       // stat()
#include <sys/un.h>         // struct sockaddr_un
#include <unistd.h>         // getcwd()
// Local includes
#include "devops_code.h"    // Headers
#include "skid_debug.h"     // PRINT_ERRNO()


#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */

// The useradd utility may only support 32 but Linux supports more
#define SKID_MAX_USERNAME_LEN 256  // Maximum username length
// The maximum lenth of an ID (e.g., UID, GID)
#define SKID_MAX_ID_LEN 10  // strlen("4294967295")

/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/
/*
 *  Description:
 *      Convert octal_value to decimal.
 *
 *  Args:
 *      octal_value: Integer representation of an octal value to convert to decimal.
 *
 *  Returns:
 *      Decimal value of octal_value.
 */
int convert_octal_to_decimal(int octal_value);

/*
 *  Description:
 *      Parser group_entry's colon-delimited fields for a particular field.
 *      This function is very fault tolerant.
 *
 *  Args:
 *      group_entry: One nul-terminated /etc/group line.  This line must be in the format
 *          specified by group(5).
 *      field_num: 0 for group_name, 1 for password, 2 for GID, or 3 for user_list.
 *
 *  Returns:
 *      A pointer into group_entry on success, NULL on invalid input.
 */
char *extract_group_field(char *group_entry, int field_num);

/*
 *  Description:
 *      Parser group_entry's colon-delimited fields for the user_list.
 *      This function is very fault tolerant.
 *
 *  Args:
 *      group_entry: One nul-terminated /etc/group line.  This line must be in the format
 *          specified by group(5).
 *
 *  Returns:
 *      A pointer into group_entry on success, NULL on invalid input.
 */
char *extract_group_user_list(char *group_entry);

/*
 *  Description:
 *      Return a heap-allocated array of all the GIDs, found in /etc/group, where username
 *      is a member.  Terminate the array with username's GID and two zero values
 *		(for double-safety).  The return value should contain all of the "groups" entries from
 *		the `id <username>` command.
 *
 *  Args:
 *      username: The name to check for compatibility.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      A heap-allocated array of compatible GIDs 0 on success.  Returns NULL on failure and
 *      errnum is updated with the relevant errno value.
 */
gid_t *parse_compatible_groups(char *username, int *errnum);

/*
 *  Description:
 *      Parser group_entry's user list for username.  This function is very fault tolerant.
 *      Trailing newlines are ignored.  Example results (sans found_gid for brevity):
 *          p_g_u_l("hark", "docker:x:134:hark") returns true
 *          p_g_u_l("hark", "nogroup:x:65534:") returns false
 *          p_g_u_l("hark", "hark:x:1337:") returns false (because the user_list is empty)
 *          p_g_u_l("hark", "docker:x:134:hark\n") returns true
 *          p_g_u_l("hark", "docker:x:134:joe,hark") returns true
 *          p_g_u_l("hark", "docker:x:134:hark,eddie") returns true
 *          p_g_u_l(NULL, "docker:x:134:hark") returns false
 *          p_g_u_l("hark", NULL) returns false
 *
 *  Args:
 *      username: The name to search /etc/group's user lists for.
 *      group_entry: One nul-terminated /etc/group line.  This line must be in the format
 *          specified by group(5).
 *      found_gid: [Out] Storage location for a compatible GID.  This value may or may not
 *          be zeroized during execution.  Ignore the value found here if this function returns
 *          false.
 *
 *  Returns:
 *      True if username if found in group_entry's properly formatted "user_list" section.
 *      False if username is not found, invalid input was received, or an error occurred.
 */
bool parse_group_user_list(char *username, char *group_entry, gid_t *found_gid);

/*
 *  Description:
 *      Read a particular colon-delimited field from group_entry into field_value.  No more than
 *      fv_len characters will be read in to field_value.  This function will nul-terminate
 *      field_value (if there's room).
 *
 *  Args:
 *      group_entry: One nul-terminated /etc/group line.  This line must be in the format
 *          specified by group(5).
 *      field_num: 0 for group_name, 1 for password, 2 for GID, or 3 for user_list.
 *      field_value: [Out] A buffer to read the group_entry field into.
 *      fv_len: The length of the buffer found at field_value.
 *
 *  Returns:
 *      0 on success, errno on invalid input or error.
 */
int read_group_field(char *group_entry, int field_num, char *field_value, int fv_len);

/*
 *  Description:
 *      Find needle in haystack.  Truncate the rest of hastack with a trailing "/\0".
 *
 *  Args:
 *      haystack: The buffer, holding an absolute directory, to search for needle in and then
 *          modify.
 *      needle: The directory name to look for in haystack.
 *      hay_len: The number of elements in haystack, to avoid buffer overruns.
 *
 *  Returns:
 *      0 on success, ENOKEY if needle is not found in haystack, ENOBUFS if haystack is not
 *      big enough to one more character, errno on error.
 */
int truncate_dir(char *haystack, const char *needle, size_t hay_len);

/*
 *  Description:
 *      Strip trailing newlines from string.  First newline character will be truncated with a
 *		nul character.
 *
 *  Args:
 *      string: A nul-terminated string to remove newlines from.
 *
 *  Returns:
 *      0 on success, EINVAL on bad input.
 */
int strip_newlines(char *string);

/*
 *  Description:
 *      Validate standard errno [Out] args on behalf of the library.  No values are changed.
 *
 *  Args:
 *      err: This must be a valid pointer.
 *
 *  Returns:
 *      0 on success, EINVAL for bad input.
 */
int validate_err(int *err);

/*
 *  Description:
 *      Validate one entry from /etc/group against the format specified in group(5).  Trailing
 *      newlines are ignored.
 *
 *  Args:
 *      name: The /etc/group entry to validate.
 *
 *  Returns:
 *      0 on success, EINVAL for bad input.
 */
int validate_group_entry(char *group_entry);

/*
 *  Description:
 *      Validate standard path names on behalf of the library.  No values are changed.
 *
 *  Args:
 *      name: This must be a valid pointer to a string that is not empty.
 *
 *  Returns:
 *      0 on success, EINVAL for bad input.
 */
int validate_name(const char *name);

/*
 *  Description:
 *      Validate standard arguments on behalf of the library.  No values are changed.
 *
 *  Args:
 *      name: This must be a valid pointer to a string that is not empty.
 *      err: This must be a valid pointer.
 *
 *  Returns:
 *      0 on success, EINVAL for bad input.
 */
int validate_standard_args(const char *name, int *err);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/
void *alloc_devops_mem(size_t num_elem, size_t size_elem, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;   // Errno value
	void *new_buf = NULL;  // Pointer to the newly allocated buffer

	// INPUT VALIDATION
	if (num_elem <= 0 || size_elem <= 0 || !errnum)
	{
		result = EINVAL;  // Invalid argument
	}

	// ALLOCATE!
	if (ENOERR == result)
	{
		new_buf = calloc(num_elem, size_elem);
		if (!new_buf)
		{
			result = errno;
			PRINT_ERROR(The call to calloc() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return new_buf;
}


int free_devops_mem(void **old_array)
{
	// LOCAL VARIABLES
	int result = ENOERR;   // Errno value
	void *old_buf = NULL;  // Pointer to the old buffer to free

	// INPUT VALIDATION
	if (old_array && *old_array)
	{
		old_buf = *old_array;
		free(old_buf);
		old_buf = NULL;
		*old_array = NULL;
	}
	else
	{
		result = EINVAL;  // NULL pointer
	}

	// DONE
	return result;
}


time_t get_shell_atime(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                    // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %X " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


blkcnt_t get_shell_block_count(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	blkcnt_t retval = 0;                  // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %b " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


gid_t *get_shell_compatible_gid(int *errnum)
{
	// LOCAL VARIABLES
	int results = ENOERR;  // Results of this function's execution
	gid_t *gid_arr = NULL;  // "My GID"-terminated, heap-allocated array of compatible GIDs
	char *my_name = NULL;   // My username

	// INPUT VALIDATION
	results = validate_err(errnum);

	// GET IT
	// 1. Get the current user's username (using get_shell_my_username())
	if (ENOERR == results)
	{
		my_name = get_shell_my_username(&results);
	}
	// 2. Parse /etc/group for all GIDs "my username" is a member of
	// 3. Store those GIDs in a heap-allocated array of gid_t values
	// 4. Terminate the array with "my username"'s GID
	if (ENOERR == results)
	{
		gid_arr = parse_compatible_groups(my_name, &results);
		if (ENOERR != results)
		{
			PRINT_ERROR(The call to parse_compatible_groups() failed);
			PRINT_ERRNO(results);
		}
	}

	// CLEANUP
	if (my_name)
	{
		free_devops_mem((void**)&my_name);  // Ignore errors
	}
	if (ENOERR != results && gid_arr)
	{
		free_devops_mem((void**)&gid_arr);  // Ignore errors
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return gid_arr;
}


time_t get_shell_ctime(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                    // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %Z " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


dev_t get_shell_device_id(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	dev_t retval = 0;                     // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %d " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


mode_t get_shell_file_perms(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	mode_t retval = 0;                    // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %a " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = convert_octal_to_decimal(atoi(output));
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


gid_t get_shell_group(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	gid_t result = 0;                     // Group ID on success
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %g " };  // The command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results
	if (!err_num)
	{
		result = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return result;
}


nlink_t get_shell_hard_links(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	nlink_t result = 0;                   // Number of hard links
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %h " };  // The command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results
	if (!err_num)
	{
		result = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return result;
}


ino_t get_shell_inode(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	ino_t result = 0;                     // Owner's ID on success
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %i " };  // The command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results
	if (!err_num)
	{
		result = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return result;
}


time_t get_shell_mtime(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                    // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %Y " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


gid_t get_shell_my_gid(int *errnum)
{
	// LOCAL VARIABLES
	gid_t my_gid = 0;              // My GID
	int results = ENOERR;          // Local errno value
	char command[] = { "id -g" };  // The shell command
	char output[512] = { 0 };      // Output from the command

	// INPUT VALIDATION
	results = validate_err(errnum);

	// GET IT
	// Execute command
	if (ENOERR == results)
	{
		results = run_command(command, output, sizeof(output) / sizeof(*output));
		if (results)
		{
			PRINT_ERROR(The call to run_command() failed);
			PRINT_ERRNO(results);
		}
	}
	// Convert results
	if (ENOERR == results)
	{
		my_gid = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return my_gid;
}


uid_t get_shell_my_uid(int *errnum)
{
	// LOCAL VARIABLES
	uid_t my_uid = 0;              // My UID
	int results = ENOERR;          // Local errno value
	char command[] = { "id -u" };  // The shell command
	char output[512] = { 0 };      // Output from the command

	// INPUT VALIDATION
	results = validate_err(errnum);

	// GET IT
	// Execute command
	if (ENOERR == results)
	{
		results = run_command(command, output, sizeof(output) / sizeof(*output));
		if (results)
		{
			PRINT_ERROR(The call to run_command() failed);
			PRINT_ERRNO(results);
		}
	}
	// Convert results
	if (ENOERR == results)
	{
		my_uid = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return my_uid;
}


char *get_shell_my_username(int *errnum)
{
	// LOCAL VARIABLES
	int results = ENOERR;                              // Local errno value
	char command[] = { "whoami" };                     // The shell command
	char username[SKID_MAX_USERNAME_LEN + 1] = { 0 };  // Local username storage
	size_t name_len = 0;                               // Length of the username
	char *name_ptr = NULL;                             // Heap-allocated buffer w/ username

	// INPUT VALIDATION
	results = validate_err(errnum);

	// GET IT
	// Execute shell command
	if (ENOERR == results)
	{
		results = run_command(command, username, sizeof(username) / sizeof(*username));
		if (results)
		{
			PRINT_ERROR(The call to run_command() failed);
			PRINT_ERRNO(results);
		}
	}
	// Cleanup username
	if (ENOERR == results)
	{
		results = strip_newlines(username);
	}
	// Measure it
	if (ENOERR == results)
	{
		name_len = strlen(username);
	}
	// Allocate a buffer
	if (ENOERR == results)
	{
		name_ptr = alloc_devops_mem(name_len + 1, sizeof(char), &results);
		if (ENOERR != results)
		{
			PRINT_ERROR(The call to alloc_devops_mem() failed);
			PRINT_ERRNO(results);
		}
	}
	// Copy username
	if (ENOERR == results)
	{
		strncpy(name_ptr, username, name_len * sizeof(char));
	}

	// CLEANUP
	if (ENOERR != results && name_ptr)
	{
		free_devops_mem((void**)&name_ptr);  // Ignore errors
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return name_ptr;
}


long get_shell_nsec_now(int *errnum)
{
	// LOCAL VARIABLES
	long retval = 0;                     // Shell command results, converted
	int err_num = ENOERR;                // Local errno value
	char base_cmd[] = { "date '+%N'" };  // The base command
	char output[512] = { 0 };            // Output from the command

	// INPUT VALIDATION
	err_num = validate_err(errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_command(base_cmd, output, sizeof(output));
	}
	// Convert results to time_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


uid_t get_shell_owner(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	uid_t result = 0;                     // Owner's ID on success
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %u " };  // The command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);
	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results
	if (!err_num)
	{
		result = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return result;
}


off_t get_shell_size(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	off_t result = 0;                     // Owner's ID on success
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %s " };  // The command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	err_num = validate_standard_args(pathname, errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_path_command(base_cmd, pathname, output, sizeof(output));
	}
	// Convert results
	if (!err_num)
	{
		result = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return result;
}


time_t get_shell_time_now(int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                   // Shell command results, converted
	int err_num = ENOERR;                // Local errno value
	char base_cmd[] = { "date '+%s'" };  // The base command
	char output[512] = { 0 };            // Output from the command

	// INPUT VALIDATION
	err_num = validate_err(errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_command(base_cmd, output, sizeof(output));
	}
	// Convert results to time_t
	if (!err_num)
	{
		retval = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


mode_t get_shell_umask(int *errnum)
{
	// LOCAL VARIABLES
	mode_t retval = 0;              // Shell command results, converted
	int err_num = ENOERR;           // Local errno value
	char base_cmd[] = { "umask" };  // The base command
	char output[512] = { 0 };       // Output from the command

	// INPUT VALIDATION
	err_num = validate_err(errnum);

	// GET IT
	// Execute command
	if (!err_num)
	{
		err_num = run_command(base_cmd, output, sizeof(output));
	}
	// Convert results to mode_t
	if (!err_num)
	{
		retval = convert_octal_to_decimal(atoi(output));
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return retval;
}


gid_t get_shell_user_gid(const char *username, int *errnum)
{
	// LOCAL VARIABLES
	gid_t users_gid = 0;            // Username's GID
	int results = ENOERR;           // Local errno value
	char command[] = { "id -g " };  // The base shell command
	char output[512] = { 0 };       // Output from the command

	// INPUT VALIDATION
	results = validate_err(errnum);

	// GET IT
	// Execute command
	if (ENOERR == results)
	{
		results = run_command_append(command, username, output, sizeof(output) / sizeof(*output));
		if (results)
		{
			PRINT_ERROR(The call to run_command_append() failed);
			PRINT_ERRNO(results);
		}
	}
	// Convert results
	if (ENOERR == results)
	{
		users_gid = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return users_gid;
}


uid_t get_shell_user_uid(const char *username, int *errnum)
{
	// LOCAL VARIABLES
	uid_t users_uid = 0;            // Username's UID
	int results = ENOERR;           // Local errno value
	char command[] = { "id -u " };  // The base shell command
	char output[512] = { 0 };       // Output from the command

	// INPUT VALIDATION
	results = validate_err(errnum);

	// GET IT
	// Execute command
	if (ENOERR == results)
	{
		results = run_command_append(command, username, output, sizeof(output) / sizeof(*output));
		if (results)
		{
			PRINT_ERROR(The call to run_command_append() failed);
			PRINT_ERRNO(results);
		}
	}
	// Convert results
	if (ENOERR == results)
	{
		users_uid = atoi(output);
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return users_uid;
}


long get_sys_block_size(int *errnum)
{
	// LOCAL VARIABLES
	long result = -1;                      // -1 on error, block size on success
	int err_num = ENOERR;                  // Local errno value
	char command[] = { "stat -fc %s ." };  // The command
	char output[512] = { 0 };              // Output from the command

	// INPUT VALIDATION
	if (errnum)
	{
		// GET IT
		// Read command results
		err_num = run_command(command, output, sizeof(output) * sizeof(*output));
		// Convert string to int
		if (!err_num)
		{
			result = atoi(output);
		}
		else
		{
			PRINT_ERROR(The call to run_command() failed);
			PRINT_ERRNO(err_num);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = err_num;
	}
	return result;
}


bool is_path_there(const char *pathname)
{
	// LOCAL VARIABLES
	bool is_there = false;  // Does pathname exist?
	int errnum = ENOERR;    // Errno value
	struct stat statbuf;    // Used in the call to stat()

	// INPUT VALIDATION
	if (pathname && *pathname)
	{
		if(stat(pathname, &statbuf))
		{
			errnum = errno;
			// ENOENT is obvious... file flat out doesn't exist.
			// ENAMETOOLONG means pathname is too long to it *can't* exist.
			// ENOTDIR means part of the path prefix of pathname is not a dir so it *can't* exist.
			if (ENOENT != errnum && ENAMETOOLONG != errnum && ENOTDIR != errnum)
			{
				if (EACCES == errnum)
				{
					// Permission denied might refer to a directory in pathname's path prefix
					PRINT_WARNG(The errno value of EACCESS is inconclusive);  // Just a warning
				}
				is_there = true;  // Other errors means it's there, even if there's a problem
			}
		}
		else
		{
			is_there = true;  // Found it
		}
	}

	// DONE
	return is_there;
}


char *join_dir_to_path(const char *dirname, const char *pathname, bool must_exist, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_standard_args(dirname, errnum);  // Results of execution
	char *joined_path = NULL;                              // dirname/pathname
	size_t total_len = 0;                                  // Total length of dirname/pathanme
	bool add_delim = false;                                // Add a / if necessary
	bool append_path = false;                              // Pathname is optional

	// INPUT VALIDATION
	// pathname
	if (ENOERR == result && pathname && *pathname)
	{
		append_path = true;  // It's there so let's append it
	}
	// dirname + must_exist
	if (ENOERR == result && true == must_exist)
	{
		if (false == is_path_there(dirname))
		{
			FPRINTF_ERR("%s - Unable to locate %s\n", DEBUG_ERROR_STR, dirname);
			result = ENOENT;  // Missing dirname
		}
	}

	// JOIN IT
	// Determine length
	if (ENOERR == result)
	{
		total_len = strlen(dirname);
		// Only add a delimiter if we need to
		if (true == append_path)
		{
			if ('/' != dirname[total_len - 1])
			{
				total_len++;  // Leave a space for the delimiter
				add_delim = true;  // Add a delimiter later
			}
			total_len += strlen(pathname);
		}
	}
	// Allocate
	if (ENOERR == result)
	{
		joined_path = alloc_devops_mem(total_len + 1, sizeof(char), &result);
		if (result)
		{
			PRINT_ERROR(The call to alloc_devops_mem() failed);
		}
	}
	// Concatenate
	if (ENOERR == result)
	{
		// dirname
		strncpy(joined_path, dirname, strlen(dirname));
		// pathname
		if (true == append_path)
		{
			// delimiter
			if (true == add_delim)
			{
				joined_path[strlen(joined_path)] = '/';
			}
			strncat(joined_path, pathname, total_len + 1);
		}
	}

	// CLEANUP
	if (result && joined_path)
	{
		free_devops_mem((void **)&joined_path);  // There was an error so let's cleanup
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return joined_path;
}


int make_a_pipe(const char *pathname)
{
	// LOCAL VARIABLES
	int result = validate_name(pathname);  // Errno value

	// INPUT VALIDATION
	if (ENOERR == result && mknod(pathname, S_IFIFO | 0664, 0))
	{
		result = errno;
		PRINT_ERROR(The call to mknod() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int make_a_socket(const char *filename)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // Errno value
	int sock_fd = -1;               // Socket file descriptor
	struct sockaddr_un local_addr;  // Struct to communicate with bind()

	// INPUT VALIDATION
	result = validate_name(filename);

	// SETUP
	if (ENOERR == result)
	{
		unlink(filename);  // Remove the socket in case it already exists
		memset(&local_addr, 0, sizeof(local_addr));  // Because the bind() man page told me to
		local_addr.sun_family = AF_UNIX;
		strncpy(local_addr.sun_path, filename, sizeof(local_addr.sun_path) - 1);
	}

	// MAKE IT
	// Open the raw local socket
	if (ENOERR == result)
	{
		sock_fd = socket(AF_UNIX, SOCK_RAW, 0);
		if (sock_fd < 0)
		{
			result = errno;
			PRINT_ERROR(The call to socket() failed);
			PRINT_ERRNO(result);
		}
	}
	// Bind it to filename (AKA "Assigning a name to a socket")
	if (ENOERR == result)
	{
		if(bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)))
		{
			result = errno;
			PRINT_ERROR(The call to bind() failed);
			PRINT_ERRNO(result);
		}
	}

	// CLEAN UP
	if (sock_fd >= 0)
	{
		close(sock_fd);
		sock_fd = -1;
	}

	// DONE
	return result;
}


int micro_sleep(useconds_t num_microsecs)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno values

	// SLEEP
	if (usleep(num_microsecs))
	{
		errnum = errno;
		PRINT_ERROR(The call to usleep() failed);
		PRINT_ERRNO(errnum);
	}

	// DONE
	return errnum;
}


char *read_a_file(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	int results = ENOERR;       // Results of this function call
	int temp_results = ENOERR;  // Temporary errno value
	char *file_buff = NULL;     // Heap-allocated buffer
	off_t file_size = 0;        // Size of filename
	FILE *file_ptr = NULL;      // Filename read stream
	size_t bytes_read = 0;      // Return value from fread()

	// INPUT VALIDATION
	results = validate_standard_args(filename, errnum);

	// READ IT
	// Size it
	if (ENOERR == results)
	{
		file_size = get_shell_size(filename, &results);
		if (results)
		{
			PRINT_ERROR(The call to get_shell_size() failed);
			PRINT_ERRNO(results);
		}
	}
	// Allocate buffer
	if (ENOERR == results)
	{
		file_buff = alloc_devops_mem(file_size + 1, 1, &results);
		if (results)
		{
			PRINT_ERROR(The call to alloc_devops_mem() failed);
			PRINT_ERRNO(results);
		}
	}
	// Open it
	if (ENOERR == results)
	{
		file_ptr = fopen(filename, "r");
		if (NULL == file_ptr)
		{
			results = errno;
			PRINT_ERROR(The call to fopen() failed);
			PRINT_ERRNO(results);
		}
	}
	// Read it
	if (ENOERR == results)
	{
		bytes_read = fread(file_buff, 1, file_size, file_ptr);
		if (bytes_read != file_size)
		{
			if (ferror(file_ptr))
			{
				PRINT_ERROR(A call to ferror() indicated an unspecified fread() error);
				results = -1;  // Unspecified error
			}
			else if (feof(file_ptr))
			{
				PRINT_WARNG(A call to feof() indicated an unanticipated EOF);  // This is fine?
			}
		}
	}

	// WRAP UP
	// FILE pointer
	if (file_ptr)
	{
		if (fclose(file_ptr))
		{
			temp_results = errno;
			PRINT_ERROR(The call to fclose() failed);
			PRINT_ERRNO(temp_results);
			if (ENOERR == results)
			{
				results = temp_results;
			}
		}
	}
	// Heap buffer
	if (ENOERR != results)
	{
		if (file_buff)
		{
			// Ignore errors since we already encountered some
			free_devops_mem((void**)&file_buff);  // Best effort
		}
	}
	// Function call results
	if (errnum)
	{
		*errnum = results;
	}

	// DONE
	return file_buff;
}


int remove_a_file(const char *filename, bool ignore_missing)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	result = validate_name(filename);
	if (ENOERR == result && remove(filename))
	{
		result = errno;
		if (ENOENT == result && true == ignore_missing)
		{
			result = ENOERR;
		}
		else
		{
			PRINT_ERROR(The call to remove() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int remove_shell_dir(const char *dirname)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // Errno value
	char command[] = { "rmdir " };  // The base shell command
	char output[512] = { 0 };       // Output from the command

	// INPUT VALIDATION
	result = validate_name(dirname);

	// GET IT
	// Execute command
	if (ENOERR == result)
	{
		result = run_command_append(command, dirname, output, sizeof(output) / sizeof(*output));
		if (result)
		{
			PRINT_ERROR(The call to run_command_append() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


char *resolve_to_repo(const char *repo_name, const char *rel_filename, bool must_exist, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;                   // Errno value
	char cwd[PATH_MAX + 1] = { 0 };        // Current working directory
	const char *tmp_file = rel_filename;   // Temp pointer to rel_filename
	char *abs_file = NULL;                 // repo_dir/rel_filename

	// INPUT VALIDATION
	result = validate_standard_args(repo_name, errnum);

	// 1. Get current working directory
	if (ENOERR == result)
	{
		if (cwd != getcwd(cwd, sizeof(cwd)))
		{
			result = errno;
			PRINT_ERROR(The call to getcwd() failed);
			PRINT_ERRNO(result);
		}
	}
	// 2. Find repo_name within the cwd, creating repo_dir
	if (ENOERR == result)
	{
		result = truncate_dir(cwd, repo_name, sizeof(cwd));
	}
	// Allocate and check
	if (ENOERR == result)
	{
		// 3. Join repo_dir to rel_filename (Optional)
		if (tmp_file && *tmp_file)
		{
			// First, advance past any leading delimiters or periods
			while (*tmp_file == '/' || *tmp_file == '.')
			{
				tmp_file++;
			}
		}
		// 4. Check must_exist
		abs_file = join_dir_to_path(cwd, tmp_file, must_exist, &result);
		if (result)
		{
			PRINT_ERROR(The call to join_dir_to_path() failed);
			PRINT_ERRNO(result);
		}
	}

	// CLEANUP
	if (ENOERR != result && abs_file)
	{
		free_devops_mem((void**)&abs_file);
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return abs_file;
}


int run_command(const char *command, char *output, size_t output_len)
{
	// LOCAL VARIABLES
	int result = ENOERR;     // Errno value
	int exit_status = 0;     // Exit status of the process
	char data[512] = { 0 };  // Temp storage for process data
	FILE *process = NULL;    // popen() stream

	// INPUT VALIDATION
	if (!command)
	{
		result = EINVAL;  // NULL pointer
	}
	else if (output && output_len <= 0)
	{
		result = EINVAL;  // Can't read into an empty buffer
	}

	// RUN IT
	// Setup read-only pipe
	if (ENOERR == result)
	{
		process = popen(command, "r");
		if (NULL == process)
		{
			result = errno;
			PRINT_ERROR(The call to popen() did not succeed);
			FPRINTF_ERR("It was popen(%s) that failed with %d\n", command, result);
			PRINT_ERRNO(result);
		}
	}
	// Get the data
	if (ENOERR == result)
	{
		if (output && output_len > 0)
		{
			fgets(output, output_len, process);
		}
		else
		{
			fgets(data, sizeof(data), process);
		}
	}

	// CLEANUP
	if (process)
	{
		exit_status = pclose(process);
		if (-1 == exit_status)
		{
			exit_status = errno;
			if (ENOERR == result)
			{
				exit_status = 1;  // What else can we do?
			}
			PRINT_ERROR(The call to pclose() encountered an error);
			PRINT_ERRNO(exit_status);
		}
		else if (exit_status)
		{
			PRINT_WARNG(Command exited with a non-zero value)
		}
		process = NULL;
	}

	// DONE
	return result;
}


int run_command_append(const char *base_cmd, const char *cmd_suffix, char *output,
	                   size_t output_len)
{
	// LOCAL VARIABLES
	char *full_cmd = NULL;     // command + cmd_suffix in a heap-allocated buffer
	size_t full_cmd_size = 0;  // Size of full_cmd buffer
	int result = ENOERR;       // Errno value

	// INPUT VALIDATION
	result = validate_name(base_cmd);
	if (ENOERR == result)
	{
		result = validate_name(cmd_suffix);
		// NOTE: The remaining arguments will be exhaustively validated by run_command()
	}

	// SETUP
	// 1. Allocate
	if (ENOERR == result && cmd_suffix && *cmd_suffix)
	{
		full_cmd_size = strlen(base_cmd) + strlen(cmd_suffix);
		full_cmd = alloc_devops_mem(full_cmd_size + 1, sizeof(char), &result);
		if (!full_cmd)
		{
			PRINT_ERROR(The call to alloc_devops_mem() encountered an error);
			if (ENOERR != result)
			{
				PRINT_ERRNO(result);
			}
			else
			{
				result = EINVAL;  // What else can we do?
			}
		}
	}
	// 2. Copy & Concatenate
	if (ENOERR == result && cmd_suffix && *cmd_suffix)
	{
		strncpy(full_cmd, base_cmd, strlen(base_cmd));
		strncat(full_cmd, cmd_suffix, strlen(cmd_suffix));
	}
	// 3. Run it
	if (ENOERR == result)
	{
		if (cmd_suffix && *cmd_suffix)
		{
			result = run_command(full_cmd, output, output_len);  // Run base_cmd + cmd_suffix
		}
		else
		{
			result = run_command(base_cmd, output, output_len);  // No suffix, just the cmd
		}
	}

	// CLEANUP
	if (full_cmd)
	{
		free_devops_mem((void**)&full_cmd);
	}

	// DONE
	return result;
}


int run_path_command(const char *command, const char *pathname, char *output, size_t output_len)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	result = validate_name(command);
	if (ENOERR == result)
	{
		result = validate_name(pathname);
		// NOTE: The remaining arguments will be exhaustively validated by run_command_append()
	}

	// SETUP
	// 1. Allocate
	if (ENOERR == result)
	{
		result = run_command_append(command, pathname, output, output_len);
	}

	// DONE
	return result;
}


int set_shell_perms(const char *pathname, mode_t new_perms)
{
	// LOCAL VARIABLES
	int result = ENOERR;                     // Errno value
	char base_cmd[64] = { "chmod " };        // The base command
	size_t base_cmd_len = strlen(base_cmd);  // Default length of the base command

	// INPUT VALIDATION
	result = validate_name(pathname);

	// FORMAT BASE COMMAND
	if (ENOERR == result)
	{
		result = snprintf(base_cmd + base_cmd_len,
			              (sizeof(base_cmd) / sizeof(*base_cmd)) - base_cmd_len, "%o ", new_perms);
		if (result < 0)
		{
			PRINT_ERROR(The call to snprintf() encountered an unspecified error);
		}
		else
		{
			result = ENOERR;  // snprintf() was just reporting on the characters it printed
		}
	}

	// EXECUTE COMMAND
	if (ENOERR == result)
	{
		result = run_path_command(base_cmd, pathname, NULL, 0);  // Ignore the output
	}

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
int convert_octal_to_decimal(int octal_value)
{
	// LOCAL VARIABLES
	int decimal_value = 0;   // Return value
	int base = 1;            // Exponential base value to 1, i.e 8^0
	int temp = octal_value;  // Temp copy of input value

	// while loop executes the statements until the
	// condition is false
	while (temp)
	{
		// Finding the last digit
		int lastdigit = temp % 10;
		temp = temp / 10;

		// Multiplying last digit with appropriate
		// base value and adding it to decimal_value
		decimal_value += lastdigit * base;

		// Increase it
		base = base * 8;
	}

	return decimal_value;
}


char *extract_group_field(char *group_entry, int field_num)
{
	// LOCAL VARIABLES
	int results = ENOERR;             // Errno value
	int count = 0;                    // Field count
	char *entry_field = group_entry;  // Pointer to the given field_num

	// INPUT VALIDATION
	results = validate_group_entry(group_entry);
	if (!results)
	{
		if (field_num < 0 || field_num > 3)
		{
			results = EINVAL;  // Bad field number
		}
	}

	// EXTRACT IT
	if (!results)
	{
		// Advance to the user_list
		while (count < field_num && *entry_field != '\0')
		{
			if (':' == *entry_field)
			{
				count++;
			}
			entry_field++;  // Advance to next character
		}
	}

	// DONE
	if (results)
	{
		entry_field = NULL;
	}
	return entry_field;
}


char *extract_group_user_list(char *group_entry)
{
	// LOCAL VARIABLES
	int results = ENOERR;      // Errno value
	char *entry_field = NULL;  // Pointer to the given field_num

	// INPUT VALIDATION
	results = validate_group_entry(group_entry);

	// EXTRACT IT
	if (!results)
	{
		entry_field = extract_group_field(group_entry, 3);
	}

	// DONE
	return entry_field;
}


gid_t *parse_compatible_groups(char *username, int *errnum)
{
	// LOCAL VARIABLES
	int results = ENOERR;             // Errno value
	gid_t local_gids[1024] = { 0 };   // Local array of GIDs
	int gid_index = 0;                // Temporary pointer into local_gids
	gid_t tmp_gid = 0;                // Temporary GID
	gid_t *gid_arr = NULL;            // Array of compatible gid_t's
	gid_t users_gid = 0;              // The GID for username
	char *group_cont = NULL;          // Contents of /etc/group
	char *tmp_curr = NULL;            // Temporary "current" line
	char *tmp_next = NULL;            // Temporary "next" line

	// INPUT VALIDATION
	results = validate_standard_args(username, errnum);

	// SETUP
	if (!results)
	{
		users_gid = get_shell_user_gid(username, &results);
	}

	// PARSE IT
	// Read /etc/group
	if (!results)
	{
		group_cont = read_a_file("/etc/group", &results);
	}
	// Parse /etc/group for all GIDs "my username" is a member of
	if (!results)
	{
		tmp_curr = group_cont;
		while (NULL != tmp_curr && '\0' != *tmp_curr)
		{
			tmp_next = strstr(tmp_curr, "\n");
			if (tmp_next)
			{
				// Separate the lines
				*tmp_next = '\0';  // Truncate that entry
				tmp_next++;  // Advance to the next entry
			}
			// At this point, tmp_curr is line N and tmp_next is line N+1 (if there is one)
			if (true == parse_group_user_list(username, tmp_curr, &tmp_gid))
			{
				if (gid_index < (sizeof(local_gids) / sizeof(*local_gids)))
				{
					// Save the user's GID for the last index
					if (tmp_gid != users_gid)
					{
						local_gids[gid_index] = tmp_gid;
						gid_index++;  // Advance to next index
					}
				}
				else
				{
					results = ENOBUFS;  // local_gids ran out of room?!
				}
			}
			// Advance the pointers to their next lines
			tmp_curr = tmp_next;  // Now pointing at N+1 (because line N has been parsed)
			tmp_next = NULL;
		}
	}
	// Truncate the local array
	if (!results)
	{
		if (gid_index < (sizeof(local_gids) / sizeof(*local_gids)))
		{
			// Save the user's GID for the last index
			local_gids[gid_index] = users_gid;
			gid_index++;  // Now this becomes the index count for the GID array
		}
		else
		{
			results = ENOBUFS;  // local_gids ran out of room?!
		}
	}
	// Allocate a heap buffer to store all the GIDs
	if (!results)
	{
		gid_arr = alloc_devops_mem(gid_index + 2, sizeof(*local_gids), &results);
	}
	// Store those GIDs in a heap-allocated array of gid_t values
	if (!results)
	{
		for (int i = 0; i < gid_index; i++)
		{
			gid_arr[i] = local_gids[i];
		}
	}

	// CLEANUP
	// Always free the /etc/group file contents
	if (group_cont)
	{
		free_devops_mem((void**)&group_cont);  // Ignore errors
	}
	// Free the GID array if it was allocated and there was an error
	if (ENOERR != results && gid_arr)
	{
		free_devops_mem((void**)&gid_arr);  // Ignore errors
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return gid_arr;
}


bool parse_group_user_list(char *username, char *group_entry, gid_t *found_gid)
{
	// LOCAL VARIABLES
	bool found_one = false;                     // Username was found in group_entry's user_list
	int results = ENOERR;                       // Control flow
	char *tmp_ptr = group_entry;                // Iterating pointer variable into group_entry
	char *found_ptr = NULL;                     // Pointer to an username in the user_list
	size_t name_len = 0;                        // Length of username
	char gid_str[SKID_MAX_ID_LEN + 1] = { 0 };  // The GID field read from group_entry

	// INPUT VALIDATION
	results = validate_name(username);
	if (!results)
	{
		results = validate_group_entry(group_entry);
	}
	if (!results && NULL == found_gid)
	{
		results = EINVAL;  // NULL gid_t pointer
	}
	else
	{
		name_len = strlen(username);
		*found_gid = 0;  // Zeroize it
	}

	// PARSE IT
	// Find a match
	if (!results)
	{
		// Advance to the user_list
		tmp_ptr = extract_group_field(group_entry, 3);
		// Check the user_list for username
		if (strlen(tmp_ptr) >= name_len)
		{
			found_ptr = strstr(tmp_ptr, username);
			if (found_ptr)
			{
				// Attempting to avoid false positives (e.g., "phil" in ":phillip")
				if ('\0' == found_ptr[name_len] || '\n' == found_ptr[name_len]
					|| ',' == found_ptr[name_len])
				{
					found_one = true;
				}
			}
		}
	}
	// Read the matching GID
	if (!results && true == found_one)
	{
		results = read_group_field(group_entry, 2, gid_str, SKID_MAX_ID_LEN);
	}
	// Store the matching GID
	if (!results && true == found_one)
	{
		*found_gid = atoi(gid_str);
	}

	// DONE
	return found_one;
}


int read_group_field(char *group_entry, int field_num, char *field_value, int fv_len)
{
	// LOCAL VARIABLES
	int results = ENOERR;      // Errno value
	char *entry_field = NULL;  // Pointer to the given field_num
	int curr_index = 0;        // Index into group_entry field number field_num

	// INPUT VALIDATION
	results = validate_group_entry(group_entry);

	// DO IT
	// Extract it
	if (!results)
	{
		entry_field = extract_group_field(group_entry, field_num);
		if (NULL == entry_field)
		{
			results = -1;  // Unspecified error
		}
	}
	// Read it
	if (!results)
	{
		while (curr_index < fv_len)
		{
			// Store it
			field_value[curr_index] = entry_field[curr_index];
			// Check it
			if (':' == field_value[curr_index] || '\n' == field_value[curr_index])
			{
				field_value[curr_index] = '\0';  // That's the end of the field
			}
			if ('\0' == field_value[curr_index])
			{
				break;  // The end
			}
			// Next
			curr_index++;  // Next group field character
		}
	}

	// DONE
	return results;
}


int strip_newlines(char *string)
{
	// LOCAL VARIABLES
	int results = ENOERR;    // Return value
	char *tmp_ptr = string;  // Iterating variable

	// INPUT VALIDATION
	results = validate_name(string);

	// STRIP IT
	if (!results)
	{
		while ('\0' != *tmp_ptr)
		{
			if ('\n' == *tmp_ptr)
			{
				*tmp_ptr = '\0';  // Truncate it
			}
			else
			{
				tmp_ptr++;  // Keep looking
			}
		}
	}

	// DONE
	return results;
}


int truncate_dir(char *haystack, const char *needle, size_t hay_len)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value
	char *temp = NULL;    // Temp pointer

	// INPUT VALIDATION
	result = validate_name(haystack);
	if (ENOERR == result)
	{
		result = validate_name(needle);
	}
	if (ENOERR == result && hay_len <= 0)
	{
		result = EINVAL;  // Invalid hay_len
	}

	// TRUNCATE IT
	// 1. Find needle in haystack
	if (ENOERR == result)
	{
		temp = strstr(haystack, needle);
		if (!temp)
		{
			if (errno)
			{
				result = errno;
			}
			else
			{
				result = ENOKEY;  // Needle not found in haystack
			}
			PRINT_ERROR(The call to strstr() did not succeed);
			PRINT_ERRNO(result);
			FPRINTF_ERR("Failed to find needle '%s' in haystack '%s'\n", needle, haystack);
		}
	}
	// 2. Verify haystack has room
	if (ENOERR == result)
	{
		temp += strlen(needle);  // Points to index after the needle
		if (hay_len < (temp - haystack + 2))
		{
			result = ENOBUFS;  // Not enough room for a trailing slash and nul char
			PRINT_WARNG(Not enough space to truncate haystack);
		}
	}
	// 3. Truncate haystack
	if (ENOERR == result)
	{
		*temp = '/';
		temp++;
		*temp = '\0';
	}

	// DONE
	return result;
}


int validate_err(int *err)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (!err)
	{
		result = EINVAL;  // Bad input
	}

	// DONE
	return result;
}


int validate_group_entry(char *group_entry)
{
	// LOCAL VARIABLES
	int result = ENOERR;          // Errno value
	int count = 0;                // Count of colon characters (of which there must be 3)
	char *tmp_ptr = group_entry;  // Iterating pointer

	// INPUT VALIDATION
	if (!group_entry || !(*group_entry))
	{
		result = EINVAL;  // Bad input
	}
	else
	{
		// Count colons
		while (*tmp_ptr != '\0' && *tmp_ptr != '\n')
		{
			if (':' == *tmp_ptr)
			{
				count++;
			}
			tmp_ptr++;  // Next character
		}
		// Validate colons
		if (3 != count)
		{
			result = EINVAL;
		}
	}

	// DONE
	return result;
}


int validate_name(const char *name)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (!name || !(*name))
	{
		result = EINVAL;  // Bad input
	}

	// DONE
	return result;
}


int validate_standard_args(const char *name, int *err)
{
	// LOCAL VARIABLES
	int result = validate_name(name);  // Errno value

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		result = validate_err(err);
	}

	// DONE
	return result;
}
