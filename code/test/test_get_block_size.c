/*
 *	Manually test skip_file_metadata_read.h's get_block_size() function.
 *
 *	Copy/paste the following...

./code/dist/test_get_block_size.bin ./code/test/test_input/regular_file.txt
./code/dist/test_get_block_size.bin ./code/test/test_input/
./code/dist/test_get_block_size.bin ./code/test/test_input/sym_link.txt

 *	NOTE: A symbolic link identifies as a regular file here because stat() follows symbolic links.
 *		Use lstat() to positively identify symbolic links.
 */

// Standard includes
#include <errno.h>                    // EINVAL
#include <stdio.h>                    // fprintf()
#include <stdlib.h>					  // exit()
// Local includes
#define SKIP_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skip_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skip_file_metadata_read.h"  // get_block_size()


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;      // Store errno and/or results here
	blksize_t answer = 0;   // Return value from get_block_size()
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
		answer = get_block_size(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_block_size() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s shows a block size of %ld.\n", pathname, (long)answer);
		}
	}

	// DONE
	exit(exit_code);
}
