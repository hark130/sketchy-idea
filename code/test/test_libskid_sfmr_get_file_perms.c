/*
 *	Manually test skid_file_metadata_read.h's get_file_perms() function...
 *  ...using the libsketchyidea shared object.
 *
 *	Copy/paste the following...

make
make install
./code/dist/test_sfmr_get_file_perms.bin ./code/test/test_input/regular_file.txt
./code/dist/test_sfmr_get_file_perms.bin ./code/test/test_input/
./code/dist/test_sfmr_get_file_perms.bin ./code/test/test_input/sym_link.txt

 *	NOTE: A symbolic link identifies as a regular file here because stat() follows symbolic links.
 *		Use lstat() to positively identify symbolic links.
 */

// Standard includes
#include <errno.h>                    // EINVAL
#include <stdint.h>					  // uintmax_t
#include <stdio.h>                    // fprintf()
#include <stdlib.h>					  // exit()
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_metadata_read.h"  // get_file_perms()


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;      // Store errno and/or results here
	mode_t answer = 0;      // Return value from get_file_perms()
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
		answer = get_file_perms(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_file_perms() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s has the following permissions: %jo (octal).\n", pathname, (uintmax_t)answer);
		}
	}

	// DONE
	exit(exit_code);
}
