/*
 *	This library defines functionality to read, parse, and report on Linux file metadata.
 */

#include "skip_file_metadata_read.h"
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/
/*
 *  Description:
 *      Validates the input arguments and updates errnum accordingly.  Will update errnum unless
 *		errnum is the cause of the problem.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      An errnor value indicating the results of validation.  0 on successful validation.
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

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


blkcnt_t get_block_count(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	blkcnt_t retval = 0;                         // Block count
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


blksize_t get_block_size(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	blksize_t retval = 0;                        // Block size
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


time_t get_change_time(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                           // Change time
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


dev_t get_container_device_id(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	dev_t retval = 0;                            // Container device id
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


dev_t get_file_device_id(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	dev_t retval = 0;                            // File device id
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


mode_t get_file_perms(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	mode_t retval = 0;                           // File permissions
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


mode_t get_file_type(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                           // File type
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


gid_t get_group(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	gid_t retval = 0;                            // GUID
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


nlink_t get_hard_link_num(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	nlink_t retval = 0;                          // Number of hard links
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


time_t get_mod_time(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	time_t retval = 0;                           // Modification time
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


uid_t get_owner(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	uid_t retval = 0;                            // UID
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


ino_t get_serial_num(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	ino_t retval = 0;                            // Inode number
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


off_t get_size(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	off_t retval = 0;                            // Size
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_block_device(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_character_device(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_directory(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(pathname, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_named_pipe(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_regular_file(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_socket(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


bool is_sym_link(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	bool retval = false;                         // Is it?
	int err = validate_input(filename, errnum);  // Errno value

	if (!err)
	{
		/* CODE GOES HERE */	
	}

	// DONE
	return retval;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
/*
 *  Description:
 *      Validates the input arguments and updates errnum accordingly.  Will update errnum unless
 *		errnum is the cause of the problem.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      An errnor value indicating the results of validation.  0 on successful validation.
 */
int validate_input(const char *filename, int *errnum)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// VALIDATE IT
	if (!filename || !(*filename) || !errnum)
	{
		retval = EINVAL;  // Invalid argument
	}

	// DONE
	if (errnum)
	{
		*errnum = retval;
	}
	return retval;
}
