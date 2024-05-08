/*
 *	Manually test skid_file_link.h's create_sym_link() function.
 *	The commands below will cleanup existing files, create symbolic links, display sym_link
 *	details/proof, and cleanup again.
 *
 *	Copy/paste the following...

# Pre-Cleanup
rm --force ./code/test/test_output/sym_dir   # Remove pre-existing symbolic link
rm --force ./code/test/test_output/sym_file  # Remove pre-existing symbolic link

# Directory
./code/dist/test_sfl_create_sym_link.bin `pwd`/code/test/test_input/ ./code/test/test_output/sym_dir; [[ $? -eq 0 ]] && \
ls -l ./code/test/test_output/sym_dir && ls ./code/test/test_output/sym_dir
# Regular file
./code/dist/test_sfl_create_sym_link.bin `pwd`/code/test/test_input/regular_file.txt ./code/test/test_output/sym_file; [[ $? -eq 0 ]] && \
ls -l ./code/test/test_output/sym_file && head -n 1 ./code/test/test_output/sym_file

# Post-Cleanup
rm --force ./code/test/test_output/sym_dir   # Remove the symbolic link
rm --force ./code/test/test_output/sym_file  # Remove the symbolic link

 *
 */

// Standard includes
#include <errno.h>                   // EINVAL
#include <stdio.h>                   // fprintf(), printf()
#include <stdlib.h>			         // exit()
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_link.h"           // create_sym_link()
#include "skid_file_metadata_read.h"  // is_sym_link()


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;         // Store errno and/or results here
	bool is_sym = false;       // Return value from is_sym_link()
	char *source_path = NULL;  // Source path
	char *dest_link = NULL;    // New symbolic link to create

	// INPUT VALIDATION
	if (argc != 3)
	{
	   fprintf(stderr, "Usage: %s <src_path> <sym_link_path>\n", argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		source_path = argv[1];
		dest_link = argv[2];
	}

	// CREATE IT
	// Create
	if (!exit_code)
	{
		exit_code = create_sym_link(source_path, dest_link);
		if (exit_code)
		{
			PRINT_ERROR(The call to create_sym_link() failed);
			PRINT_ERRNO(exit_code);
		}
	}
	// Verify
	if (!exit_code)
	{
		is_sym = is_sym_link(dest_link, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to is_sym_link() failed);
			PRINT_ERRNO(exit_code);
		}
		else if (false == is_sym)
		{
			FPRINTF_ERR("%s - Everything succeeded but '%s' is not a symbolic link\n",
				        DEBUG_WARNG_STR, dest_link);
		}
		else
		{
			fprintf(stdout, "Successfully created '%s' as a symbolic link to '%s'\n",
				    dest_link, source_path);
		}
	}

	// DONE
	exit(exit_code);
}
