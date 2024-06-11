/*
 *	This library defines functionality to help manager server/clients.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO(), PRINT_ERROR()
#include "skid_network.h"				// SKID_BAD_FD

#ifdef SKID_DEBUG
#include <stdio.h>   // fprintf()
// getaddrinfo() returns unique non-zero exit codes.  The gai_strerror() function translates
// these error codes to a human readable string, suitable for error reporting.
// See: getaddrinfo(3)
#define PRINT_GAI_ERR(errcode) if (errcode) { fprintf(stderr, "%s - %s - %s() - line %d - Returned getaddressinfo error code [%d]: %s\n", DEBUG_ERROR_STR, __FILE__, __FUNCTION_NAME__, __LINE__, errcode, gai_strerror(errcode)); };
#else
#define PRINT_GAI_ERR(errcode) ;;;
#endif  /* SKID_DEBUG */

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
		if (bind(sockfd, addr, addrlen))
		{
			result = errno;
			PRINT_ERROR(The call to socket() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int close_socket(int sockfd, bool quiet)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// CLOSE IT
	if (close(sockfd) && false == quiet)
	{
		result = errno;
		PRINT_ERROR(The call to close() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int free_addr_info(struct addrinfo **res)
{
	// LOCAL VARIABLES
	int result = ENOERR;                // Errno values
	struct addrinfo *head_node = NULL;  // Head node of the addrinfo struct pointer linked list

	// INPUT VALIDATION
	if (NULL == res || NULL == *res)
	{
		result = EINVAL;  // NULL pointer
	}
	// FREE IT
	else
	{
		freeaddrinfo(*res);  // Free it
		*res = NULL;  // Zeroize it
	}

	// DONE
	return result;
}


int get_addr_info(const char *node, const char *service, const struct addrinfo *hints,
	              struct addrinfo **res)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values
	int gai_errcode = 0;  // getaddressinfo() error code

	// INPUT VALIDATION
	// node && service
	if (NULL == node && NULL == service)
	{
		result = EINVAL;  // Either node or service, but not both, may be NULL
	}
	// res
	else if (NULL == res)
	{
		result = EINVAL;  // NULL pointer
	}

	// GET IT
	if (ENOERR == result)
	{
		errno = ENOERR;  // Reset errno
		gai_errcode = getaddrinfo(node, service, hints, res);
		if (0 != gai_errcode)
		{
			result = errno;
			PRINT_ERROR(The call to getaddrinfo() failed);
			PRINT_GAI_ERR(gai_errcode);
			if (ENOERR == result)
			{
				result = -1;  // Unspecified error
			}
			else
			{
				PRINT_ERRNO(result);
			}
		}
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
		if (listen(sockfd, backlog))
		{
			result = errno;
			PRINT_ERROR(The call to listen() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int open_socket(int domain, int type, int protocol, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno values
	int sockfd = SKID_BAD_FD;  // Socket file descriptor

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

	// CLEANUP
	if (ENOERR != result)
	{
		close(sockfd, true);  // Close the file descriptor but ignore errors
		sockfd = SKID_BAD_FD;  // Ensure compliance with the function's documented behavior
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
