/*
 *	Manually test skip_file_metadata_read.h's get_file_device_id() function.
 *
 *	Copy/paste the following...

./code/dist/test_sfmr_get_file_device_id.bin ./code/test/test_input/regular_file.txt
./code/dist/test_sfmr_get_file_device_id.bin ./code/test/test_input/
./code/dist/test_sfmr_get_file_device_id.bin ./code/test/test_input/sym_link.txt

 *	NOTE: A symbolic link identifies as a regular file here because stat() follows symbolic links.
 *		Use lstat() to positively identify symbolic links.
 */

// Standard includes
#include <errno.h>                    // EINVAL
#include <stdio.h>                    // fprintf(), printf()
#include <stdlib.h>					  // exit()
#include <sys/sysmacros.h>			  // major(), minor()
// Local includes
#define SKIP_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skip_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skip_file_metadata_read.h"  // get_file_device_id()


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;      // Store errno and/or results here
	dev_t answer = 0;       // Return value from get_file_device_id()
	char *pathname = NULL;  // Get this from argv[1]

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

	// CHECK IT
	if (!exit_code)
	{
		answer = get_file_device_id(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_file_device_id() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s has a raw device ID of: %d.\n", pathname, (int)answer);
			printf("\tMajor device ID: %d\n", major(answer));
			printf("\tMinor device ID: %d\n", minor(answer));
		}
	}

	// DONE
	exit(exit_code);
}
