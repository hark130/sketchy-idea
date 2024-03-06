/*
 *	Manually test skip_file_metadata_read.h's is_named_pipe() function.
 *
 *	Copy/paste the following...

mkfifo ./code/test/test_input/named_pipe
./code/dist/test_is_named_pipe.bin ./code/test/test_input/regular_file.txt  # No
./code/dist/test_is_named_pipe.bin ./code/test/test_input/                  # No
./code/dist/test_is_named_pipe.bin ./code/test/test_input/sym_link.txt      # No
./code/dist/test_is_named_pipe.bin ./code/test/test_input/named_pipe        # Yes
rm ./code/test/test_input/named_pipe

 *
 */

// Standard includes
#include <errno.h>                    // EIO
#include <stdio.h>                    // fprintf()
#include <stdlib.h>					  // exit()
// Local includes
#define SKIP_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skip_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skip_file_metadata_read.h"  // is_named_pipe()


#define TEST_FILE_TYPE (const char *)"named pipe"  // Use this in copy/paste test cases


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;      // Store errno and/or results here
	bool answer = false;    // Return value from is_named_pipe()
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
		answer = is_named_pipe(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to is_named_pipe() failed);
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
