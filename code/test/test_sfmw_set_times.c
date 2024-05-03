/*
 *	Manually test skid_file_metadata_write.h's set_times() functions.
 *	The second argument is what the atime and mtime timestamps will be changed to.  Specifically,
 *	it is represented as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
 *	(See: time(2))
 *
 *	Copy/paste the following...

./code/dist/test_sfmw_set_times.bin ./code/test/test_input/ 1712946852                  # Directory
./code/dist/test_sfmw_set_times.bin ./code/test/test_input/regular_file.txt 1712946852  # Regular file

 *
 *	NOTE: Adding 1712946852 seconds to epoch time should result in 4/12/2024, 1:34:12 PM
 */

// Standard includes
#include <errno.h>                     // EINVAL
#include <stdio.h>                     // fprintf(), printf()
#include <stdlib.h>					   // exit()
#include <sys/sysmacros.h>			   // major(), minor()
// Local includes
#define SKID_DEBUG                     // The DEBUG output is doing double duty as test output
#include "skid_debug.h"                // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_metadata_read.h"   // get_*_time()
#include "skid_file_metadata_write.h"  // set_times()


/*
 *	skid_file_metadata_read's get_*_time() functions generally follow this behavior:
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch the time for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *		follow_sym: If false, uses lstat() for symlinks.
 *
 *  Returns:
 *		The time, on success.  0 on error, and errnum is set.
 */
typedef time_t (*GetTime)(const char *pathname, int *errnum, bool follow_sym);


/*
 *  Description:
 *		Print the human readable atime, ctime, and mtime for pathname.
 *      
 *  Args:
 *      pathname: The path to fetch times for.
 *		follow_sym: If true, follows symlinks to evaluate the target.
 *      
 *  Returns:
 *      On success, 0.  Errno or -1 on failure.
 */
int print_all_times(const char *pathname, bool follow_sym);


/*
 *  Description:
 *		Call get_time_func(), translate the result into a human-readable string, and print results.
 *      
 *  Args:
 *      get_time_func: Function pointer to a get_*_time() function.
 *		func_type: The answer to "...pathname's _____ is..." (e.g., access, change, modification)
 *      pathname: The path to fetch a time for.
 *		follow_sym: If true, follows symlinks to evaluate the target.
 *      
 *  Returns:
 *      On success, 0.  Errno or -1 on failure.
 */
int print_formatted_time(GetTime get_time_func, const char *func_type, const char *pathname,
	                     bool follow_sym);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;              // Store errno and/or results here
	char *pathname = NULL;          // Get this from argv[1]
	time_t num_secs = 0;            // Converted "number of seconds" argument value
	bool follow_sym_links = false;  // Follow symbolic links

	// INPUT VALIDATION
	if (argc != 3)
	{
	   fprintf(stderr, "Usage: %s <pathname> <num_of_seconds>\n", argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		pathname = argv[1];
		num_secs = atoi(argv[2]);
	}

	// GET IT
	// Before
	if (!exit_code)
	{
		puts("BEFORE");
		exit_code = print_all_times(pathname, follow_sym_links);
	}
	// Set time
	if (!exit_code)
	{
		exit_code = set_times(pathname, follow_sym_links, num_secs, 0);
		if (exit_code)
		{
			PRINT_ERROR(The call to set_times() failed);
			PRINT_ERRNO(exit_code);
		}
	}
	// After
	if (!exit_code)
	{
		puts("AFTER");
		exit_code = print_all_times(pathname, follow_sym_links);
	}

	// DONE
	exit(exit_code);
}


int print_all_times(const char *pathname, bool follow_sym)
{
	// LOCAL VARIABLES
	int result = 0;  // Store errno and/or results here

	// GET IT
	// Access Time (atime)
	if (!result)
	{
		result = print_formatted_time(get_access_time, "access", pathname, follow_sym);
	}
	// Change Time (ctime)
	if (!result)
	{
		result = print_formatted_time(get_change_time, "change", pathname, follow_sym);
	}
	// Modification Time (mtime)
	if (!result)
	{
		result = print_formatted_time(get_mod_time, "modification", pathname, follow_sym);
	}

	// DONE
	return result;
}


int print_formatted_time(GetTime get_time_func, const char *func_type, const char *pathname,
	                     bool follow_sym)
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
		answer = (*get_time_func)(pathname, &result, follow_sym);
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
