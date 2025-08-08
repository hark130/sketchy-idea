/*
 *	Manually test skid_dir_operations' read_dir_contents().
 *
 *	Copy/paste the following...

./code/dist/test_sdo_read_dir_contents.bin <DIR_TO_READ>

 *
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include <errno.h>						// EINVAL
#include <stdlib.h>						// exit()
#include "skid_debug.h"					// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_dir_operations.h"		// read_dir_contents()
#include "skid_file_metadata_read.h"	// is_directory()
#include "skid_macros.h"				// ENOERR

/*
 *	Description:
 *		Print num_entries number of entries from dir_contents.
 *
 *	Args:
 *		dir_name: The name of the directory the contents were read from.
 *		dir_contents: An NULL-terminate array of string pointers, on success.
 *			May be NULL (for an empty dir).
 *		num_entries: The total number of indices, 0 or otherwise, in dir_contents.
 *
 *	Returns:
 *		ENOERR on success, errno on failure.  EIO is used to indicate a mismatch between the
 *		NULL-terminated dir_contents and num_entries.
 */
int print_contents(char *dir_name, char **dir_contents, size_t num_entries);

/*
 *	Description:
 *		Print the "help" usage for this manual test code.
 *
 *	Args:
 *		prog_name: Pass in argv[0] here.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;           // Store errno and/or results here
	bool recurse = false;        // Recurse argument value for read_dir_contents()
	char **dir_contents = NULL;  // Pointer to an array of character pointers
	size_t num_entries = 0;      // Number of entries in the dir_contents array

	// INPUT VALIDATION
	if (argc != 2)
	{
		print_usage(argv[0]);
		exit_code = EINVAL;
	}
	else if (false == is_directory(argv[1], &exit_code))
	{
		fprintf(stderr, "'%s' is not a directory.\n%s\n", argv[1], strerror(exit_code));
		print_usage(argv[0]);
		if (ENOERR == exit_code)
		{
			exit_code = ENOTDIR;  // Just in case
		}
	}
	
	// READ IT
	if (ENOERR == exit_code)
	{
		dir_contents = read_dir_contents(argv[1], recurse, &exit_code, &num_entries);
		if (ENOERR == exit_code)
		{
			exit_code = print_contents(argv[1], dir_contents, num_entries);
		}
		else
		{
			PRINT_ERROR(The call to read_dir_contents() failed);
			PRINT_ERRNO(exit_code);
		}
	}

	// CLEANUP
	free_skid_dir_contents(&dir_contents);  // Best effort

	// DONE
	exit(exit_code);
}


int print_contents(char *dir_name, char **dir_contents, size_t num_entries)
{
	// LOCAL VARIABLES
	int errnum = ENOERR;    // Errno value to return
	char **tmp_ptr = NULL;  // Iterating variable
	int count = 0;          // Number of entries found in dir_contents

	// INPUT VALIDATION
	if (NULL == dir_name || 0x0 == *dir_name)
	{
		errnum = EINVAL;  // Bad string
	}
	if (NULL == dir_contents && 0 != num_entries)
	{
		FPRINTF_ERR("Directory contents are empty but num_entries is %lu\n", num_entries);
		errnum = EIO;  // Mismatch
	}
	else if (NULL != dir_contents && 0 == num_entries)
	{
		FPRINTF_ERR("Directory contents exist but num_entries is %lu\n", num_entries);
		errnum = EIO;  // Mismatch
	}
	else
	{
		// Validate length
		tmp_ptr = dir_contents;
		while (tmp_ptr && *tmp_ptr)
		{
			count++;  // Increment the count
			tmp_ptr++;  // Next position
		}
		if (count >= num_entries && count > 0)
		{
			FPRINTF_ERR("Directory content count is %d but num_entries is %lu\n",
				        count, num_entries);
			errnum = EIO;  // Mismatch
		}
	}

	// PRINT CONTENTS
	if (ENOERR == errnum)
	{
		printf("The contents of '%s':\n", dir_name);
		if (NULL == dir_contents)
		{
			printf("\t[EMPTY]\n");
		}
		else
		{
			for (int i = 0; i < num_entries && dir_contents[i]; i++)
			{
				printf("\t%s\n", dir_contents[i]);
			}
		}
	}

	// DONE
	return errnum;
}


void print_usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s <DIRECTORY>\n", prog_name);
}
