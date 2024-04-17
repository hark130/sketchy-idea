/*
 *	This library contains non-releasable, unit-test-specific, miscellaneous helper code.
 */

#include <errno.h>      	// errno
#include <limits.h>			// PATH_MAX
#include <stdio.h>			// remove()
#include <stdlib.h>			// calloc(), free()
#include <string.h>			// strstr()
#include <sys/socket.h>		// AF_UNIX, socket()
#include <sys/stat.h>		// stat()
#include <sys/un.h>			// struct sockaddr_un
#include <unistd.h>			// getcwd()
// Local includes
#include "devops_code.h"	// Headers
#include "skid_debug.h"		// PRINT_ERRNO()


#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */

// The useradd utility may only support 32 but Linux supports more
#define SKID_MAX_USERNAME_LEN 256  // Maximum username length

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
 *      Find needle in haystack.  Truncate the rest of hastack with a trailing "/\0".
 *
 *  Args:
 *      haystack: The buffer, holding an absolute directory, to search for needle in and then
 *			modify.
 *		needle: The directory name to look for in haystack.
 *		hay_len: The number of elements in haystack, to avoid buffer overruns.
 *
 *  Returns:
 *      0 on success, ENOKEY if needle is not found in haystack, ENOBUFS if haystack is not
 *		big enough to one more character, errno on error.
 */
int truncate_dir(char *haystack, const char *needle, size_t hay_len);

/*
 *  Description:
 *      Validate standard errno [Out] args on behalf of the library.  No values are changed.
 *
 *  Args:
 *		err: This must be a valid pointer.
 *
 *  Returns:
 *      0 on success, EINVAL for bad input.
 */
int validate_err(int *err);

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
 *		err: This must be a valid pointer.
 *
 *  Returns:
 *      0 on success, EINVAL for bad input.
 */
int validate_standard_args(const char *name, int *err);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/
char *alloc_devops_mem(size_t num_elem, size_t size_elem, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;   // Errno value
	char *new_buf = NULL;  // Pointer to the newly allocated buffer

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


int free_devops_mem(char **old_array)
{
	// LOCAL VARIABLES
	int result = ENOERR;   // Errno value
	char *old_buf = NULL;  // Pointer to the old buffer to free

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
	if (ENOERR == result)
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
	if (ENOERR == result)
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
		else
		{
			name_len = strlen(username);
		}
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
	if (ENOERR != resuls && name_ptr)
	{
		free_devops_mem(&name_ptr);  // Ignore errors
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return name_ptr;
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
			free_devops_mem(&file_buff);  // Best effort
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


char *resolve_to_repo(const char *repo_name, const char *rel_filename, bool must_exist, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;                   // Errno value
	char cwd[PATH_MAX + 1] = { 0 };        // Current working directory
	const char *tmp_file = rel_filename;   // Temp pointer to rel_filename
	char *abs_file = NULL;                 // repo_dir/rel_filename
	struct stat sb;                        // Used by stat()


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
	// 3. Join repo_dir to rel_filename (Optional)
	if (ENOERR == result && tmp_file && *tmp_file)
	{
		// First, advance past any leading delimiters or periods
		while (*tmp_file == '/' || *tmp_file == '.')
		{
			tmp_file++;
		}
		// Next, append rel_filename to cwd
		if (*tmp_file)
		{
			strncat(cwd, tmp_file, sizeof(cwd) - strlen(cwd));
		}
	}
	// 4. Check must_exist
	if (ENOERR == result && must_exist == true)
	{
		if (stat(cwd, &sb))
		{
			result = errno;
			PRINT_WARNG(The status of the resolved path is uncertain);
			PRINT_ERRNO(result);
		}
	}
	// 5. Allocate heap memory
	if (ENOERR == result)
	{
		abs_file = alloc_devops_mem(strlen(cwd) + 1, sizeof(*cwd), &result);
	}
	// 6. Copy the local absolute path into the heap memory
	if (ENOERR == result)
	{
		strncpy(abs_file, cwd, strlen(cwd));
	}

	// CLEANUP
	if (ENOERR != result && abs_file)
	{
		free_devops_mem(&abs_file);
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
		if (!process)
		{
			result = errno;
		}
		PRINT_ERROR(The call to popen() did not succeed);
		PRINT_ERRNO(result);
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


int run_command_append(const char *base_cmd, const char *cmd_suffix, char *output, size_t output_len)
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
		free_devops_mem(&full_cmd);
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
			PRINT_ERROR(The call to strcasestr() did not succeed);
			PRINT_ERRNO(result);
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
