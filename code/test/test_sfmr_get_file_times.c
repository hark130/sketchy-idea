/*
 *	Manually test skid_file_metadata_read.h's get_*_time() functions.
 *
 *	Copy/paste the following...

./code/dist/test_sfmr_get_file_times.bin /dev/loop0                               # Block device
./code/dist/test_sfmr_get_file_times.bin /dev/null                                # Char device
./code/dist/test_sfmr_get_file_times.bin ./code/test/test_input/                  # Directory
./code/dist/test_sfmr_get_file_times.bin ./code/test/test_input/regular_file.txt  # Regular file
./code/dist/test_sfmr_get_file_times.bin /var/run/dbus/system_bus_socket          # Socket
./code/dist/test_sfmr_get_file_times.bin ./code/test/test_input/sym_link.txt      # Sym link

 *	NOTE: A symbolic link identifies as a regular file here because stat() follows symbolic links.
 *		Use lstat() to positively identify symbolic links.
 */

// Standard includes
#include <errno.h>                    // EINVAL
#include <stdio.h>                    // fprintf(), printf()
#include <stdlib.h>					  // exit()
#include <sys/sysmacros.h>			  // major(), minor()
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_metadata_read.h"  // get_*_time()


/*
 *	skid_file_metadata_read's get_*_time() functions generally follow this behavior:
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch the time for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		The time, on success.  0 on error, and errnum is set.
 */
typedef time_t (*GetTime)(const char *pathname, int *errnum);


/*
 *  Description:
 *		Call get_time_func(), translate the result into a human-readable string, and print results.
 *      
 *  Args:
 *      get_time_func: Function pointer to a get_*_time() function.
 *		func_type: The answer to "...pathname's _____ is..." (e.g., access, change, modification)
 *      pathname: The path to fetch a time for.
 *      
 *  Returns:
 *      On success, 0.  Errno or -1 on failure.
 */
int print_formatted_time(GetTime get_time_func, const char *func_type, const char *pathname);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;               // Store errno and/or results here
	char *pathname = NULL;           // Get this from argv[1]

	// INPUT VALIDATION
	if (argc != 2)
	{
	   fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		pathname = argv[1];
	}

	// GET IT
	// Access Time (atime)
	if (!exit_code)
	{
		exit_code = print_formatted_time(get_access_time, "access", pathname);
	}
	// Change Time (ctime)
	if (!exit_code)
	{
		exit_code = print_formatted_time(get_change_time, "change", pathname);
	}
	// Modification Time (mtime)
	if (!exit_code)
	{
		exit_code = print_formatted_time(get_mod_time, "modification", pathname);
	}

	// DONE
	exit(exit_code);
}


int print_formatted_time(GetTime get_time_func, const char *func_type, const char *pathname)
{
	// LOCAL VARIABLES
	int result = 0;              // Store errno and/or results here
	time_t answer = 0;           // Return value from get_time_func()
	char time_str[512] = { 0 };  // Temp storage buffer for human-readable time string

	// INPUT VALIDATION
	if (!get_time_func || !func_type || !(*func_type) || !pathname || !(*pathname))
	{
		result = EINVAL;  // Bad input
	}

	// PRINT IT
	// Get the time
	if (!result)
	{
		answer = (*get_time_func)(pathname, &result);
	}
	// Format the time
	if (!result)
	{
		result = format_time(time_str, sizeof(time_str) - 1, answer);
	}
	// Print the time
	if (!result)
	{
		printf("%s's %s time is: %s\n", pathname, func_type, time_str);
	}

	// DONE
	return result;
}
