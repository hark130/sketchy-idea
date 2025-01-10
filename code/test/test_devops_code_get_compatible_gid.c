/*
 *	Manually test skid_file_metadata_write.h's get_shell_compatible_gid() functions.
 *	The commands below will set the ownership to your current UID and GID.
 *	Adjust the values as necessary.
 *
 *	Copy/paste the following...

./code/dist/test_devops_code_get_compatible_gid.bin
sudo ./code/dist/test_devops_code_get_compatible_gid.bin
sudo -u nobody ./code/dist/test_devops_code_get_compatible_gid.bin
sudo -u whoopsie ./code/dist/test_devops_code_get_compatible_gid.bin

 *
 */

// Standard includes
#include <errno.h>                     // EINVAL
#include <stdio.h>                     // fprintf(), printf()
#include <stdlib.h>					   // exit()
#include <sys/sysmacros.h>			   // major(), minor()
// Local includes
#include "devops_code.h"			   // get_shell_compatible_gid(), get_shell_my_uid()
#ifndef SKID_DEBUG
#define SKID_DEBUG                     // The DEBUG output is doing double duty as test output
#endif  /* SKID_DEBUG */
#include "skid_debug.h"                // PRINT_ERRNO(), PRINT_ERROR()


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;      // Store errno and/or results here
	uid_t my_uid = 0;       // The UID of the executing process
	gid_t *gid_arr = NULL;  // Array of compatible GIDs
	gid_t *tmp_ptr = NULL;  // Temp iterating variable for the GID array
	gid_t tmp_gid = 0;      // Watch for duplicates to avoid buffer overruns

	// INPUT VALIDATION
	if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s\n", argv[0]);
	   exit_code = EINVAL;
	}

	// GET IT
	// Get my UID
	if (!exit_code)
	{
		my_uid = get_shell_my_uid(&exit_code);
		printf("My UID is %u\n", my_uid);
	}
	// Get compatible GIDs
	if (!exit_code)
	{
		gid_arr = get_shell_compatible_gid(&exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_shell_compatible_gid() failed);
			PRINT_ERRNO(exit_code);
		}
		// printf("%s Ownership:\tUID=%u   GID=%u\n", pathname, curr_uid, curr_gid);
	}
	// Print compatible GIDs
	if (!exit_code)
	{
		tmp_ptr = gid_arr;
		printf("Compatible GIDs:");
		while(tmp_gid != my_uid)
		{
			if (tmp_gid != *tmp_ptr)
			{
				printf("\t%u", *tmp_ptr);
				tmp_gid = *tmp_ptr;
				tmp_ptr++;
			}
			else
			{
				break;  // Found a duplicate so it must be done
			}
		}
		printf("\n");
		tmp_ptr = NULL;
	}

	// CLEANUP
	if (gid_arr)
	{
		exit_code = free_devops_mem((void**)&gid_arr);
	}

	// DONE
	exit(exit_code);
}
