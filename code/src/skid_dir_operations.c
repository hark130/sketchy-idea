/*
 *	This library defines functionality to create, remove, and parse Linux directories.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO()
#include "skid_dir_operations.h"		// delete_dir()
#include <errno.h>						// errno
#include <unistd.h>						// link(), symlink()
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/



/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int create_dir(const char *dirname, mode_t mode)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// INPUT VALIDATION
	// TO DO: DON'T DO NOW

	// DELETE IT
	// TO DO: DON'T DO NOW

	// DONE
	return result;
}


int delete_dir(const char *dirname)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// INPUT VALIDATION
	// TO DO: DON'T DO NOW

	// DELETE IT
	// TO DO: DON'T DO NOW

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


