/*
 *	This library defines functionality to allocate and free memory on behalf of SKID.
 */

// #define SKID_DEBUG						// Enable DEBUG logging

#include <errno.h>						// errno
#include <stdlib.h>						// calloc()
#include <string.h>						// strlen()
#include "skid_debug.h"				  	// PRINT_ERRNO()
#include "skid_memory.h"				// free_skid_string()
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *	Description:
 *		Validate common arguments on behalf of skid_memory.
 *
 *	Args:
 *		pathname: A non-NULL, non-empty, pathname.
 *		err: A non-NULL integer pointer.
 *
 *	Returns:
 *		0 for good input, errno for failed validation.
 */
int validate_sm_standard_args(const char *pathname, int *err);

/*
 *	Description:
 *		Validate errno out paramters on behalf of skid_memory.
 *
 *	Args:
 *		err: A non-NULL integer pointer.
 *
 *	Returns:
 *		0 for good input, errno for failed validation.
 */
int validate_sm_err(int *err);

/*
 *	Description:
 *		Validate pathnames on behalf of skid_memory.
 *
 *	Args:
 *		pathname: A non-NULL, non-empty, pathname.
 *
 *	Returns:
 *		0 for good input, errno for failed validation.
 */
int validate_sm_pathname(const char *pathname);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


void *alloc_skid_mem(size_t num_elem, size_t size_elem, int *errnum)
{
	// LOCAL VARIABLES
	void *new_mem = NULL;  // Heap allocated memory
	int result = EINVAL;   // Store local errno values here

	// INPUT VALIDATION
	if (num_elem > 0 && size_elem > 0 && errnum)
	{
		result = ENOERR;  // Looks good
	}

	// ALLOCATE IT
	if (ENOERR == result)
	{
		new_mem = calloc(num_elem, size_elem);
		if (!new_mem)
		{
			result = errno;
			PRINT_ERROR(The call to calloc() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return new_mem;
}


char *copy_skid_string(const char *source, int *errnum)
{
	// LOCAL VARIABLES
	char *destination = NULL;  // Heap-allocated copy of source
	int result = ENOERR;       // Errno values
	size_t src_len = 0;        // Length of source

	// INPUT VALIDATION
	result = validate_sm_standard_args(source, errnum);

	// COPY IT
	// Size it
	if (ENOERR == result)
	{
		src_len = strlen(source);
	}
	// Allocate
	if (ENOERR == result)
	{
		destination = alloc_skid_mem(src_len + 1, sizeof(char), &result);
	}
	// Copy
	if (ENOERR == result)
	{
		strncpy(destination, source, src_len);
	}

	// CLEANUP
	if (result)
	{
		free_skid_string(&destination);  // Best effort
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return destination;
}


int free_skid_mem(void **old_mem)
{
	// LOCAL VARIABLES
	int result = EINVAL;  // Results from execution

	// INPUT VALIDATION
	if (old_mem && *old_mem)
	{
		result = ENOERR;
	}

	// FREE IT
	if (ENOERR == result)
	{
		free(*old_mem);
		*old_mem = NULL;
	}

	// DONE
	return result;
}


int free_skid_string(char **old_string)
{
	// LOCAL VARIABLES
	int result = free_skid_mem((void **)old_string);  // Results from execution

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int validate_sm_standard_args(const char *pathname, int *err)
{
	// LOCAL VARIABLES
	int result = validate_sm_pathname(pathname);  // Store errno value

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		result = validate_sm_err(err);
	}

	// DONE
	return result;
}


int validate_sm_err(int *err)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Store errno value

	// INPUT VALIDATION
	if (!err)
	{
		result = EINVAL;  // NULL pointer
	}

	// DONE
	return result;
}


int validate_sm_pathname(const char *pathname)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Store errno value

	// INPUT VALIDATION
	if (!pathname || !(*pathname))
	{
		result = EINVAL;  // Bad pathname
	}

	// DONE
	return result;
}
