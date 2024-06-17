/*
 *	This library defines functionality to manage Linux file descriptors.
 */

#include "skid_file_descriptors.h"		// close_fd()
#include "skid_macros.h"	// SKID_BAD_FD


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *	Description:
 *		Validate file descriptors on behalf of the library.
 *
 *	Args:
 *		fd: File descriptor to validate.
 *
 *	Returns:
 *		0 on success, errno on failed validation. 
 */
int validate_sfd_fd(int fd);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int close_fd(int *fdp, bool quiet)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// INPUT VALIDATION
	if (NULL == fdp)
	{
		result = EINVAL;  // NULL pointer
	}
	else
	{
		result = validate_sfd_fd(*fdp);
	}

	// CLOSE IT
	if (ENOERR == result)
	{
		if (close(*fdp))
		{
			// close() failed
			result = errno;
			if (false == quiet)
			{
				PRINT_ERROR(The call to close() failed);
				PRINT_ERRNO(result);
			}
		}
		else
		{
			*fdp = SKID_BAD_FD;
		}
	}

	// DONE
	return result;
}


char *read_fd(int fd, int *errnum)
{
	// TO DO: DON'T DO NOW... define this function and implement in simple_stream_server.c
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/

int validate_sfd_fd(int fd)
{
	// LOCAL VARIABLES
	int result = EBADF;  // Validation result

	// INPUT VALIDATION
	if (fd >= 0)
	{
		result = ENOERR;  // Good(?).
	}

	// DONE
	return result;
}
