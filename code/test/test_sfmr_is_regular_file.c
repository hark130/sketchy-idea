/*
 *	Manually test skip_file_metadata_read.h's is_regular_file() function.
 *
 *	Copy/paste the following...

./code/dist/test_sfmr_is_regular_file.bin ./code/test/test_input/regular_file.txt  # Yes
./code/dist/test_sfmr_is_regular_file.bin ./code/test/test_input/                  # No
./code/dist/test_sfmr_is_regular_file.bin ./code/test/test_input/sym_link.txt      # Yes, for reasons

 *	NOTE: A symbolic link identifies as a regular file here because stat() follows symbolic links.
 *		Use lstat() to positively identify symbolic links.
 */

// Standard includes
#include <errno.h>                    // EIO
#include <stdio.h>                    // fprintf()
#include <stdlib.h>					  // exit()
// Local includes
#define SKIP_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skip_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skip_file_metadata_read.h"  // is_regular_file()


#define TEST_FILE_TYPE (const char *)"regular file"  // Use this in copy/paste test cases


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;      // Store errno and/or results here
	bool answer = false;    // Return value from is_regular_file()
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
		answer = is_regular_file(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to is_regular_file() failed);
			PRINT_ERRNO(exit_code);
		}
		else if (true == answer)
		{
			printf("%s is a %s.\n", pathname, TEST_FILE_TYPE);
		}
		else if (false == answer)
		{
			printf("%s is *NOT* a %s!\n", pathname, TEST_FILE_TYPE);
		}
		else
		{
			exit_code = EIO;  // As good a choice as anything, I suppose
			PRINT_ERROR(How did we get here?);
		}
	}

	// DONE
	exit(exit_code);
}
