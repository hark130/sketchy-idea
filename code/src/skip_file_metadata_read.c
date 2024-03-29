/*
 *	This library defines functionality to read, parse, and report on Linux file metadata.
 */

#include "skid_debug.h"				  // PRINT_ERRNO()
#include "skid_file_metadata_read.h"
#include <string.h>					  // memset()
#include <time.h>					  // localtime(), strftime()
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */

#define SKID_DEBUG  // Enable DEBUGGING output

/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/
/*
 *  Description:
 *      Calls lstat(pathname) and updates statbuf.  Standardizes basic error handling.  Updates
 *		errnum with any errno values encountered, 0 on success.
 *  Args:
 *      pathname: Absolute or relative pathname to check with lstat().
 *		statbuf: [Out] Pointer to a stat struct to update with the results of the call to stat().
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int call_lstat(const char *pathname, struct stat *statbuf, int *errnum);

/*
 *  Description:
 *      Calls stat(pathname) and updates statbuf.  Standardizes basic error handling.  Updates
 *		errnum with any errno values encountered, 0 on success.
 *  Args:
 *      pathname: Absolute or relative pathname to check with stat().
 *		statbuf: [Out] Pointer to a stat struct to update with the results of the call to stat().
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int call_stat(const char *pathname, struct stat *statbuf, int *errnum);

/*
 *  Description:
 *      Validates the input arguments and updates errnum accordingly.  Will update errnum unless
 *		errnum is the cause of the problem.
 *  Args:
 *      filename: Must be non-NULL and also can't be empty.
 *		statbuf: Must be a non-NULL pointer.
 *      errnum: Must be a non-NULL pointer.  Set to 0 on success.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_call_input(const char *pathname, struct stat *statbuf, int *errnum);

/*
 *  Description:
 *      Validates the input arguments and updates errnum accordingly.  Will update errnum unless
 *		errnum is the cause of the problem.
 *  Args:
 *      filename: Must be non-NULL and also can't be empty.
 *      errnum: Must be a non-NULL pointer.  Set to 0 on success.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_input(const char *filename, int *errnum);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/
time_t get_access_time(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                           // Access time
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		retval = stat_struct.st_atime;
	}

	// DONE
	return retval;
}


blkcnt_t get_block_count(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	blkcnt_t retval = 0;                         // Block count
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		retval = stat_struct.st_blocks;
	}

	// DONE
	return retval;
}


blksize_t get_block_size(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	blksize_t retval = 0;                        // Block size
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		// The st_blksize field gives the "preferred" blocksize for efficient file system I/O.
		retval = stat_struct.st_blksize;
	}

	// DONE
	return retval;
}


time_t get_change_time(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                           // Change time
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		retval = stat_struct.st_ctime;
	}

	// DONE
	return retval;
}


dev_t get_container_device_id(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	dev_t retval = 0;                            // Container device id
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_rdev;
	}

	// DONE
	return retval;
}


dev_t get_file_device_id(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	dev_t retval = 0;                            // File device id
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_dev;
	}

	// DONE
	return retval;
}


mode_t get_file_perms(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	mode_t perm_mask = S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO;  // Perm bitmask
	mode_t retval = 0;                                                             // File perms
	int err = validate_input(filename, errnum);                                    // Errno value
	struct stat stat_struct;                                                       // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_mode & perm_mask;
	}

	// DONE
	return retval;
}


mode_t get_file_type(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	mode_t retval = 0;                           // File type
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		// S_IFMT is the bit mask for the file type bit field
		retval = stat_struct.st_mode & S_IFMT;
	}

	// DONE
	return retval;
}


gid_t get_group(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	gid_t retval = 0;                            // GUID
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_gid;
	}

	// DONE
	return retval;
}


nlink_t get_hard_link_num(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	nlink_t retval = 0;                          // Number of hard links
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_nlink;
	}

	// DONE
	return retval;
}


time_t get_mod_time(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                           // Modification time
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		retval = stat_struct.st_mtime;
	}

	// DONE
	return retval;
}


uid_t get_owner(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	uid_t retval = 0;                            // UID
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_uid;
	}

	// DONE
	return retval;
}


ino_t get_serial_num(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	ino_t retval = 0;                            // Inode number
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_ino;
	}

	// DONE
	return retval;
}


off_t get_size(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	off_t retval = 0;                            // Size
	int err = validate_input(pathname, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// GET IT
	// Fetch metadata
	if (!err)
	{
		err = call_stat(pathname, &stat_struct, errnum);
	}
	// Get it
	if (!err)
	{
		retval = stat_struct.st_size;
	}

	// DONE
	return retval;
}


int format_time(char *output, size_t output_size, time_t time_val)
{
	// LOCAL VARIABLES
	int result = ENOERR;                      // Result of execution
	struct tm *tmp;                           // time_val translated to local time
	time_t time_copy = time_val;              // Local copy of time_val
	char format[] = { "%Y-%m-%d %H:%M:%S" };  // Standard format for time

	// INPUT VALIDATION
	if (!output || output_size <= 0)
	{
		result = EINVAL;  // Bad input
	}

	// SETUP
	if (ENOERR == result)
	{
		memset(output, 0x0, output_size);  // Clear it
		errno = ENOERR;  // Zeroize it, for safety
		tmp = localtime(&time_copy);  // Populate the tm struct
		result = errno;  // localtime() may result in EOVERFLOW
		if (result)
		{
			PRINT_ERROR(The call to localtime() failed);
			PRINT_ERRNO(result);
		}
	}

	// FORMAT IT
	if (ENOERR == result)
	{
		if (0 == strftime(output, output_size, format, tmp))
		{
			PRINT_ERROR(The call to strftime() returned 0);
			result = -1;
		}
	}

	// DONE
	return result;
}


bool is_block_device(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	// IS IT?
	if (!err)
	{
		if (S_IFBLK == get_file_type(filename, errnum))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


bool is_character_device(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	// IS IT?
	if (!err)
	{
		if (S_IFCHR == get_file_type(filename, errnum))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


bool is_directory(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(pathname, errnum);  // Errno value

	// IS IT?
	if (!err)
	{
		if (S_IFDIR == get_file_type(pathname, errnum))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


bool is_named_pipe(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	// IS IT?
	if (!err)
	{
		if (S_IFIFO == get_file_type(filename, errnum))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


bool is_regular_file(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	// IS IT?
	if (!err)
	{
		if (S_IFREG == get_file_type(filename, errnum))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


bool is_socket(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	// IS IT?
	if (!err)
	{
		if (S_IFSOCK == get_file_type(filename, errnum))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


bool is_sym_link(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value
	struct stat stat_struct;                     // stat struct

	// IS IT?
	// Fetch metadata
	if (!err)
	{
		err = call_lstat(filename, &stat_struct, errnum);
	}
	// Check it
	if (!err)
	{
		if (S_IFLNK == (stat_struct.st_mode & S_IFMT))
		{
			retval = true;
		}
	}

	// DONE
	return retval;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
int call_lstat(const char *pathname, struct stat *statbuf, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_call_input(pathname, statbuf, errnum);  // Errno value

	// CALL LSTAT
	if (ENOERR == result)
	{
		if (lstat(pathname, statbuf))
		{
			result = errno;
			PRINT_ERROR(The call to lstat() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return result;
}


int call_stat(const char *pathname, struct stat *statbuf, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_call_input(pathname, statbuf, errnum);  // Errno value

	// CALL STAT
	if (ENOERR == result)
	{
		if (stat(pathname, statbuf))
		{
			result = errno;
			PRINT_ERROR(The call to stat() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return result;
}


int validate_call_input(const char *pathname, struct stat *statbuf, int *errnum)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// VALIDATE IT
	// pathname
	// errnum
	retval = validate_input(pathname, errnum);
	// statbuf
	if (ENOERR == retval)
	{
		if (!statbuf)
		{
			retval = EINVAL;  // Invalid argument
			PRINT_ERROR(Invalid Argument - Received a null statbuf pointer);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = retval;
	}
	return retval;
}


int validate_input(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// VALIDATE IT
	// pathname
	if (!pathname)
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received a null pathname pointer);
	}
	else if (!(*pathname))
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received an empty pathname);
	}
	// errnum
	if (!errnum)
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received a null errnum pointer);
	}

	// DONE
	if (errnum)
	{
		*errnum = retval;
	}
	return retval;
}
