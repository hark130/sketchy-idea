/*
 *	This library contains non-releasable, unit-test-specific, miscellaneous helper code.
 */

#include <errno.h>      	// errno
#include <limits.h>			// PATH_MAX
#include <stdio.h>			// remove()
#include <stdlib.h>			// calloc(), free()
#include <string.h>			// strstr()
#include <sys/stat.h>		// stat()
#include <unistd.h>			// getcwd()
// Local includes
#include "devops_code.h"	// Headers
#include "skip_debug.h"		// PRINT_ERRNO()


#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */

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


blkcnt_t get_shell_block_count(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	blkcnt_t retval = 0;                  // Shell command results, converted
	int err_num = ENOERR;                 // Local errno value
	char base_cmd[] = { "stat -c %b " };  // The base command
	char output[512] = { 0 };             // Output from the command

	// INPUT VALIDATION
	if (!pathname || !(*pathname) || !errnum)
	{
		err_num = EINVAL;  // Bad input
	}

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
	if (!pathname || !(*pathname) || !errnum)
	{
		err_num = EINVAL;  // Bad input
	}

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
	if (!pathname || !(*pathname) || !errnum)
	{
		err_num = EINVAL;  // Bad input
	}

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
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (!pathname || !(*pathname))
	{
		result = EINVAL;  // Invalid input
	}
	else if (mknod(pathname, S_IFIFO | 640, 0))
	{
		result = errno;
		PRINT_ERROR(The call to mknod() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int remove_a_file(const char *filename, bool ignore_missing)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (!filename || !(*filename))
	{
		result = EINVAL;  // Invalid input
	}
	else if (remove(filename))
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
	if (!repo_name || !(*repo_name))
	{
		result = EINVAL;  // Bad repo_name
	}
	else if (!errnum)
	{
		result = EINVAL;  // Bad errnum
	}

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


int run_path_command(const char *command, const char *pathname, char *output, size_t output_len)
{
	// LOCAL VARIABLES
	char *full_cmd = NULL;     // command + pathname in a heap-allocated buffer
	size_t full_cmd_size = 0;  // Size of full_cmd buffer
	int result = ENOERR;       // Errno value

	// INPUT VALIDATION
	if (!command || !pathname || !(*pathname))
	{
		result = EINVAL;  // Invalid pathname
		// NOTE: The remaining arguments will be exhaustively validated by run_command()
	}

	// SETUP
	// 1. Allocate
	if (ENOERR == result)
	{
		full_cmd_size = strlen(command) + strlen(pathname);
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
	if (ENOERR == result)
	{
		strncpy(full_cmd, command, strlen(command));
		strncat(full_cmd, pathname, strlen(pathname));
	}
	// 3. Run it
	if (ENOERR == result)
	{
		result = run_command(full_cmd, output, output_len);
	}

	// CLEANUP
	if (full_cmd)
	{
		free_devops_mem(&full_cmd);
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
	if (!haystack || !(*haystack) || !needle || !(*needle) || hay_len <= 0)
	{
		result = EINVAL;  // Invalid argument
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
