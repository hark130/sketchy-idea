/*
 *	Manually test most of skid_file_metadata_read.h's functions.
 *
 *	Copy/paste the following...

./code/dist/test_sfmr_file_metadata.bin /dev/loop0                               # Block device
./code/dist/test_sfmr_file_metadata.bin /dev/null                                # Char device
./code/dist/test_sfmr_file_metadata.bin ./code/test/test_input/                  # Directory
./code/dist/test_sfmr_file_metadata.bin ./code/test/test_input/regular_file.txt  # Regular file
./code/dist/test_sfmr_file_metadata.bin /var/run/dbus/system_bus_socket          # Socket
./code/dist/test_sfmr_file_metadata.bin ./code/test/test_input/sym_link.txt      # Sym link

 *	NOTE: A symbolic link identifies as a regular file here because stat() follows symbolic links.
 *		Use lstat() to positively identify symbolic links.
 */

// Standard includes
#include <errno.h>                    // EINVAL
#include <stdio.h>                    // fprintf(), printf()
#include <stdlib.h>					  // exit()
#include <sys/sysmacros.h>			  // major(), minor()
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_metadata_read.h"  // get_*_time()


/*
 *	skid_file_metadata_read's get_*_time() functions generally follow this behavior:
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch the time for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		The time, on success.  0 on error, and errnum is set.
 */
typedef time_t (*GetTime)(const char *pathname, int *errnum);


/*
 *  Description:
 *		Print pathname's file type in a standard way.
 *
 *  Args:
 *      pathname: The path to fetch a time for.
 *
 *  Returns:
 *      On success, 0.  Errno on failure.
 */
int print_file_type(const char *pathname);


/*
 *  Description:
 *		Call get_time_func(), translate the result into a human-readable string, and print results.
 *
 *  Args:
 *      get_time_func: Function pointer to a get_*_time() function.
 *		func_type: The answer to "...pathname's _____ is..." (e.g., access, change, modification)
 *      pathname: The path to fetch a time for.
 *
 *  Returns:
 *      On success, 0.  Errno or -1 on failure.
 */
int print_formatted_time(GetTime get_time_func, const char *func_type, const char *pathname);


/*
 *  Description:
 *		Print pathname's owner and group IDs in a standard way.
 *
 *  Args:
 *      pathname: The path to fetch ownership for.
 *
 *  Returns:
 *      On success, 0.  Errno on failure.
 */
int print_ownership(const char *pathname);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;               // Store errno and/or results here
	char tmp_time_str[512] = { 0 };  // Temp storage buffer for human-readable time string
	time_t tmp_time = 0;             // Return value from get_*_time()
	dev_t tmp_dev = 0;               // Temp variable for library return values
	ino_t tmp_ino = 0;               // Temp variable for library return values
	mode_t tmp_mode = 0;             // Temp variable for library return values
	nlink_t tmp_link = 0;            // Temp variable for library return values
	blksize_t tmp_blksize = 0;       // Temp variable for library return values
	off_t tmp_off = 0;               // Temp variable for library return values
	blkcnt_t tmp_blkcnt = 0;         // Temp variable for library return values
	char *pathname = NULL;           // Get this from argv[1]

	// INPUT VALIDATION
	if (argc != 2)
	{
	   FPRINTF_ERR("Usage: %s <pathname>\n", argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		pathname = argv[1];
		printf("\nFETCHING METADATA FOR %s\n", pathname);
	}

	// GET IT
	// Device ID
	if (!exit_code)
	{
		tmp_dev = get_file_device_id(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_file_device_id() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("Device ID:\n");
			printf("\tMajor device ID: %d\n", major(tmp_dev));
			printf("\tMinor device ID: %d\n", minor(tmp_dev));
		}
	}
	// Containder Device ID
	if (!exit_code)
	{
		tmp_dev = get_container_device_id(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_container_device_id() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			if (tmp_dev)
			{
				printf("However, the device denoted by %s has a raw device ID of: %d.\n", pathname,
					   (int)tmp_dev);
				printf("\tMajor device ID: %d\n", major(tmp_dev));
				printf("\tMinor device ID: %d\n", minor(tmp_dev));
			}
			else
			{
				FPRINTF_ERR("%s does not denote a character or block special file.\n", pathname);
			}
		}
	}
	// File Type
	if (!exit_code)
	{
		exit_code = print_file_type(pathname);
	}
	// Inode Number
	if (!exit_code)
	{
		tmp_ino = get_serial_num(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_serial_num() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("Inode number: %ld\n", tmp_ino);
		}
	}
	// Permissions
	if (!exit_code)
	{
		tmp_mode = get_file_perms(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_file_perms() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("Mode: %o\n", tmp_mode);
		}
	}
	if (!exit_code)
	{
		tmp_link = get_hard_link_num(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_hard_link_num() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("Number of hard links: %ld\n", tmp_link);
		}
	}
	// Owner and Group IDs
	if (!exit_code)
	{
		exit_code = print_ownership(pathname);
	}
	// Preferred Block Size
	if (!exit_code)
	{
		tmp_blksize = get_block_size(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_block_size() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("Preferred I/O block size: %ld.\n", (long)tmp_blksize);
		}
	}
	// File Size
	if (!exit_code)
	{
		tmp_off = get_size(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_size() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("File Size: %ld.\n", tmp_off);
		}
	}
	// Blocks Allocated
	if (!exit_code)
	{
		tmp_blkcnt = get_block_count(pathname, &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to get_size() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("Blocks Allocated: %ld.\n", tmp_off);
		}
	}
	// Access Time (atime)
	if (!exit_code)
	{
		exit_code = print_formatted_time(get_access_time, "access", pathname);
	}
	// Change Time (ctime)
	if (!exit_code)
	{
		exit_code = print_formatted_time(get_change_time, "status change", pathname);
	}
	// Modification Time (mtime)
	if (!exit_code)
	{
		exit_code = print_formatted_time(get_mod_time, "modification", pathname);
	}

	// DONE
	printf("\n");
	exit(exit_code);
}


int print_file_type(const char *pathname)
{
	// LOCAL VARIABLES
	int result = 0;              // Store errno and/or results here

	// PRINT IT
	printf("File type: ");

	if (!result)
	{
		if (true == is_sym_link(pathname, &result))
		{
			printf("symbolic link to a ");
		}
	}
	if (!result)
	{
		if (true == is_block_device(pathname, &result))
		{
			printf("block device\n");
		}
	}
	if (!result)
	{
		if (true == is_character_device(pathname, &result))
		{
			printf("character device\n");
		}
	}
	if (!result)
	{
		if (true == is_directory(pathname, &result))
		{
			printf("directory\n");
		}
	}
	if (!result)
	{
		if (true == is_named_pipe(pathname, &result))
		{
			printf("FIFO\n");
		}
	}
	if (!result)
	{
		if (true == is_regular_file(pathname, &result))
		{
			printf("regular file\n");
		}
	}
	if (!result)
	{
		if (true == is_socket(pathname, &result))
		{
			printf("socket\n");
		}
	}

	// DONE
	return result;
}


int print_formatted_time(GetTime get_time_func, const char *func_type, const char *pathname)
{
	// LOCAL VARIABLES
	int result = 0;              // Store errno and/or results here
	time_t answer = 0;           // Return value from get_time_func()
	char time_str[512] = { 0 };  // Temp storage buffer for human-readable time string

	// INPUT VALIDATION
	if (!get_time_func || !func_type || !(*func_type) || !pathname || !(*pathname))
	{
		result = EINVAL;  // Bad input
	}

	// PRINT IT
	// Get the time
	if (!result)
	{
		answer = (*get_time_func)(pathname, &result);
	}
	// Format the time
	if (!result)
	{
		result = format_time(time_str, sizeof(time_str) - 1, answer);
	}
	// Print the time
	if (!result)
	{
		printf("%s time is: %s\n", func_type, time_str);
	}

	// DONE
	return result;
}


int print_ownership(const char *pathname)
{
	// LOCAL VARIABLES
	int result = 0;     // Store errno and/or results here
	uid_t tmp_uid = 0;  // Return value from library function calls
	gid_t tmp_gid = 0;  // Return value from library function calls

	// PRINT IT
	// Get the owner ID
	tmp_uid = get_owner(pathname, &result);
	if (result)
	{
		PRINT_ERROR(The call to get_owner() failed);
		PRINT_ERRNO(result);
	}
	else
	{
		printf("Ownership:\n\tOwner ID (%u)\n", tmp_uid);
	}
	// Get the group ID
	if (!result)
	{
		tmp_gid = get_group(pathname, &result);
		if (result)
		{
			PRINT_ERROR(The call to get_group() failed);
			PRINT_ERRNO(result);
		}
		else
		{
			printf("\tGroup ID (%u)\n", tmp_gid);
		}
	}

	// DONE
	return result;
}
