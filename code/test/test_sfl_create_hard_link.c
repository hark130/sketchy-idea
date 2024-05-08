/*
 *	Manually test skid_file_link.h's create_hard_link() function.
 *	The commands below will cleanup existing files, create symbolic links, display sym_link
 *	details/proof, and cleanup again.
 *
 *	Copy/paste the following...

# Pre-Cleanup
rm --force ./code/test/test_output/hard_dir   # Remove pre-existing symbolic link
rm --force ./code/test/test_output/hard_file  # Remove pre-existing symbolic link

# Regular file
./code/dist/test_sfl_create_hard_link.bin `pwd`/code/test/test_input/regular_file.txt ./code/test/test_output/hard_file; [[ $? -eq 0 ]] && \
ls -l ./code/test/test_output/hard_file && head -n 1 ./code/test/test_output/hard_file

# Post-Cleanup
rm --force ./code/test/test_output/hard_dir   # Remove the symbolic link
rm --force ./code/test/test_output/hard_file  # Remove the symbolic link

 *
 */

// Standard includes
#include <errno.h>                   // EINVAL
#include <stdint.h>					 // uintmax_t
#include <stdio.h>                   // fprintf(), printf()
#include <stdlib.h>			         // exit()
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_link.h"           // create_hard_link()
#include "skid_file_metadata_read.h"  // get_hard_link_num(), is_sym_link()


/*
 *  Description:
 *		Print the number of hard links to pathname with a caveat: "%s: %s has %d hard links".
 *      
 *  Args:
 *      pathname: The path to count hard links for.
 *		caveat: [Optional] Intended to be "BEFORE" or "AFTER" but it can be NULL.
 *      
 *  Returns:
 *      On success, 0.  Errno or -1 on failure.
 */
int print_num_hard_links(const char *pathname, const char *caveat);


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
	   fprintf(stderr, "Usage: %s <src_path> <hard_link_path>\n", argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		source_path = argv[1];
		dest_link = argv[2];
	}

	// DO IT
	// Print "BEFORE" number of hard links
	if (!exit_code)
	{
		exit_code = print_num_hard_links(source_path, "BEFORE");
	}
	// Create it
	if (!exit_code)
	{
		exit_code = create_hard_link(source_path, dest_link);
		if (exit_code)
		{
			PRINT_ERROR(The call to create_hard_link() failed);
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
		else if (true == is_sym)
		{
			FPRINTF_ERR("%s - Everything succeeded but '%s' registers as a symbolic link\n",
				        DEBUG_WARNG_STR, dest_link);
		}
		else
		{
			fprintf(stdout, "Successfully created '%s' as a hard link to '%s'\n",
				    dest_link, source_path);
		}
	}
	// Print "AFTER" number of hard links
	if (!exit_code)
	{
		exit_code = print_num_hard_links(source_path, "AFTER");
	}

	// DONE
	exit(exit_code);
}


int print_num_hard_links(const char *pathname, const char *caveat)
{
	// LOCAL VARIABLES
	int results = 0;               // Results of execution
	nlink_t num_hard_links = 0;    // Number of hard links to pathname
	const char *descriptor = "";   // Before, After, NULL, etc.
	const char *punctuation = "";  // Dynamic punctuation

	// SETUP
	if (caveat && *caveat)
	{
		descriptor = caveat;
		punctuation = ": ";
	}

	// PRINT IT
	num_hard_links = get_hard_link_num(pathname, &results);
	if (results)
	{
		PRINT_ERROR(The call to get_hard_link_num() failed);
		PRINT_ERRNO(results);
	}
	else
	{
		fprintf(stdout, "%s%s%s has %ju hard link(s)\n", descriptor, punctuation,
			    pathname, (uintmax_t)num_hard_links);
	}

	// DONE
	return results;
}
