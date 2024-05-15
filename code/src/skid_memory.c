/*
 *	This library defines functionality to allocate and free memory on behalf of SKID.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include <errno.h>						// errno
#include <stdlib.h>						// calloc()
#include "skid_debug.h"				  	// PRINT_ERRNO()
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/



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


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
