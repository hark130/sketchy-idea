/*
 *	Manually test skid_file_metadata_write.h's set_ownership() functions.
 *	The commands below will set set the ownership to your current UID and GID.
 *	Adjust the values as necessary.
 *
 *	Copy/paste the following...

./code/dist/test_sfmw_set_ownership.bin ./code/test/test_input/ `id -u` `id -g`                  # Directory
./code/dist/test_sfmw_set_ownership.bin ./code/test/test_input/regular_file.txt `id -u` `id -g`  # Regular file

 *
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
#include "skid_file_metadata_write.h"  // set_ownership()


/*
 *  Description:
 *		Print the UID and GID for pathname.
 *      
 *  Args:
 *      pathname: The path to fetch the IDs for.
 *      
 *  Returns:
 *      On success, 0.  Errno or -1 on failure.
 */
int print_all_ids(const char *pathname);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;              // Store errno and/or results here
	char *pathname = NULL;          // Get this from argv[1]
	uid_t new_uid = 0;              // Converted UID argument value
	gid_t new_gid = 0;              // Converted GID argument value
	bool follow_sym_links = false;  // Follow symbolic links

	// INPUT VALIDATION
	if (argc != 4)
	{
	   fprintf(stderr, "Usage: %s <pathname> <new_UID> <new_GID>\n"
	   	       "NOTE: Use -1 as an ID value to skip it.\n", argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		pathname = argv[1];
		new_uid = atoi(argv[2]);
		new_gid = atoi(argv[3]);
	}

	// GET IT
	// Before
	if (!exit_code)
	{
		puts("BEFORE");
		exit_code = print_all_ids(pathname);
	}
	// Set time
	if (!exit_code)
	{
		exit_code = set_ownership(pathname, new_uid, new_gid, follow_sym_links);
		if (exit_code)
		{
			PRINT_ERROR(The call to set_ownership() failed);
			PRINT_ERRNO(exit_code);
		}
	}
	// After
	if (!exit_code)
	{
		puts("AFTER");
		exit_code = print_all_ids(pathname);
	}

	// DONE
	exit(exit_code);
}


int print_all_ids(const char *pathname)
{
	// LOCAL VARIABLES
	int result = 0;  	 // Store errno and/or results here
	uid_t curr_uid = 0;  // Current UID
	gid_t curr_gid = 0;  // Current GID

	// GET IT
	// UID
	if (!result)
	{
		curr_uid = get_owner(pathname, &result);
		if (result)
		{
			PRINT_ERROR(The call to get_owner() failed);
			PRINT_ERRNO(result);
		}
	}
	// GID
	if (!result)
	{
		curr_gid = get_group(pathname, &result);
		if (result)
		{
			PRINT_ERROR(The call to get_group() failed);
			PRINT_ERRNO(result);
		}
	}

	// PRINT IT
	if (!result)
	{
		printf("%s Ownership:\tUID=%u   GID=%u\n", pathname, curr_uid, curr_gid);
	}

	// DONE
	return result;
}
