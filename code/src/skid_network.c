/*
 *	This library defines functionality to help manager server/clients.
 */
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Validates the errnum arguments on behalf of this library.
 *
 *  Args:
 *      err: A non-NULL pointer to an integer.
 *
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_sn_err(int *err);

/*
 *	Description:
 *		Validate socket file descriptors on behalf of the library.
 *
 *	Args:
 *		sockfd: Socket file descriptor to validate.
 *
 *	Returns:
 *		0 on success, errno on failed validation. 
 */
int validate_sn_sockfd(int sockfd);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int bind_struct(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// INPUT VALIDATION
	result = validate_sn_sockfd(sockfd);

	// BIND IT
	if (ENOERR == result)
	{

	}

	// DONE
	return result;
}


int listen_socket(int sockfd, int backlog)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// INPUT VALIDATION
	result = validate_sn_sockfd(sockfd);

	// LISTEN TO IT
	if (ENOERR == result)
	{

	}

	// DONE
	return result;
}


int open_socket(int domain, int type, int protocol, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values
	int sockfd = -1;      // Socket file descriptor

	// INPUT VALIDATION
	result = validate_sn_err(errnum);

	// OPEN IT
	if (ENOERR == result)
	{
		errno = ENOERR;  // Clear errno... for safety
		sockfd = socket(domain, type, protocol);
		result = errno;
		// Validate results
		if (ENOERR != result)
		{
			PRINT_ERROR(The call to socket() failed);
			PRINT_ERRNO(result);
			sockfd = -1;  // Ensure compliance with the function's comment block
		}
		else
		{
			result = validate_sn_sockfd(sockfd);
			if (ENOERR != result)
			{
				PRINT_ERROR(The call to socket() returned an invalid file descriptor);
				PRINT_ERRNO(result);
			}
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return sockfd;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int validate_sn_err(int *err)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // The results of validation

	// INPUT VALIDATION
	if (!err)
	{
		result = EINVAL;  // NULL pointer
	}

	// DONE
	return result;
}


int validate_sn_sockfd(int sockfd)
{
	// LOCAL VARIABLES
	int result = EBADF;  // Validation result

	// INPUT VALIDATION
	if (sockfd >= 0)
	{
		result = ENOERR;  // Good(?).
	}

	// DONE
	return result;
}
