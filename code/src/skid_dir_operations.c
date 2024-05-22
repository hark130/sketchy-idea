/*
 *	This library defines functionality to create, remove, and parse Linux directories.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO()
#include "skid_dir_operations.h"		// delete_dir()
#include "skid_file_metadata_read.h"	// is_directory()
#include <errno.h>						// errno
#include <unistd.h>						// link(), rmdir(), symlink()
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Validates the pathname arguments on behalf of this library.
 *  Args:
 *      pathname: A non-NULL pointer to a non-empty string.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_sdo_pathname(const char *pathname);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int create_dir(const char *dirname, mode_t mode)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// INPUT VALIDATION
	result = validate_sdo_pathname(dirname);

	// DELETE IT
	if (ENOERR == result)
	{
		if (mkdir(dirname, mode))
		{
			result = errno;
			PRINT_ERROR(The call to mkdir() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int delete_dir(const char *dirname)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution
	bool is_dir = false;  // Is dirname a directory that exists?

	// INPUT VALIDATION
	result = validate_sdo_pathname(dirname);
	if (ENOERR == result)
	{
		is_dir = is_directory(dirname, &result);
		if (false == is_dir && ENOERR == result)
		{
			result = ENOTDIR;  // It exists, just not as a directory
		}
	}

	// DELETE IT
	if (ENOERR == result)
	{
		if (rmdir(dirname))
		{
			result = errno;
			PRINT_ERROR(The call to rmdir() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int destroy_dir(const char *dirname)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// INPUT VALIDATION
	// TO DO: DON'T DO NOW

	// DESTROY IT
	// TO DO: DON'T DO NOW

	// DONE
	return result;
}


char **read_dir_contents(const char *dirname, bool recurse, int *errnum)
{
	// LOCAL VARIABLES
	char **content_arr = NULL;  // NULL-terminated array of nul-terminated strings
	int result = ENOERR;        // Capture errno values here

	// INPUT VALIDATION
	// TO DO: DON'T DO NOW

	// READ IT
	// TO DO: DON'T DO NOW

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return content_arr;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int validate_sdo_pathname(const char *pathname)
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

	// DONE
	return retval;
}
