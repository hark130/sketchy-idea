/*
 *	This library defines functionality to help manager server/clients.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"					// PRINT_ERRNO(), PRINT_ERROR()
#include "skid_network.h"				// SKID_BAD_FD
#include <arpa/inet.h>					// inet_ntop()
#include <errno.h>						// EINVAL
#include <unistd.h>						// close()

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
 *	Description:
 *		Retrieve the relevant address pointer based on the struct's sa_family value.
 *
 *	Args:
 *		sa: Pointer to the struct to evaluate.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		Pointer to the relevant struct in*_addr member on success.  NULL on error (consult
 *		errnum for details).  EPFNOSUPPORT is used to indicate an unsupported sa->sa_family.
 */
void *get_inet_addr(struct sockaddr *sa, int *errnum);

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


int accept_client(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;          // Errno values
	int client_fd = SKID_BAD_FD;  // Accepted client file descriptor

	// INPUT VALIDATION
	result = validate_sn_sockfd(sockfd);
	if (ENOERR == result)
	{
		if (!(NULL == addr) != !(NULL == addrlen))
		{
			result = EINVAL;  // If addr is NULL, so should addrlen (and vice versa)
		}
	}

	// ACCEPT IT
	if (ENOERR == result)
	{
		client_fd = accept(sockfd, addr, addrlen);
		if (client_fd < 0)
		{
			result = errno;
			PRINT_ERROR(The call to accept() failed);
			PRINT_ERRNO(result);
			client_fd = SKID_BAD_FD;  // Ensuring compliance with function documentation
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return client_fd;
}


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


int close_socket(int *sockfd, bool quiet)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// INPUT VALIDATION
	if (NULL == sockfd)
	{
		result = EINVAL;  // NULL pointer
	}
	else
	{
		result = validate_sn_sockfd(*sockfd);
	}

	// CLOSE IT
	if (ENOERR == result)
	{
		if (close(*sockfd))
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
			*sockfd = SKID_BAD_FD;
		}
	}

	// DONE
	return result;
}


int connect_socket(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// INPUT VALIDATION
	result = validate_sn_sockfd(sockfd);

	// CONNECT IT
	if (ENOERR == result)
	{
		if (connect(sockfd, addr, addrlen))
		{
			result = errno;
			PRINT_ERROR(The call to connect() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int convert_sas_ip(struct sockaddr_storage *addr, char *ip_buff, size_t ip_size)
{
	// LOCAL VARIABLES
	int result = ENOERR;     // Errno values
	void *inet_addr = NULL;  // Extracted pointer of the relevant struct in*_addr member

	// INPUT VALIDATION
	if (NULL == addr || NULL == ip_buff || ip_size <= 0)
	{
		result = EINVAL;  // Bad input
	}

	// CONVERT IT
	// Get the in_addr member
	if (ENOERR == result)
	{
		inet_addr = get_inet_addr((struct sockaddr *)addr, &result);
	}
	// Convert in_addr member
	if (ENOERR == result)
	{
		if (ip_buff != inet_ntop(addr->ss_family, inet_addr, ip_buff, ip_size))
		{
			result = errno;
			PRINT_ERROR(The call to inet_ntop() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int free_addr_info(struct addrinfo **res)
{
	// LOCAL VARIABLES
	int result = ENOERR;                // Errno values

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
		close_socket(&sockfd, true);  // Close the file descriptor but ignore errors
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


void *get_inet_addr(struct sockaddr *sa, int *errnum)
{
	// LOCAL VARIABLES
	void *inet_addr = NULL;  // Pointer to the relevant struct in*_addr member
	int result = ENOERR;     // Errno values

	// INPUT VALIDATION
	result = ( NULL == sa ) ? EINVAL : validate_sn_err(errnum);

	// GET IT
	if (ENOERR == result)
	{
		if (AF_INET == sa->sa_family)
		{
			inet_addr = (void *)&(((struct sockaddr_in*)sa)->sin_addr);
		}
		else if (AF_INET6 == sa->sa_family)
		{
			inet_addr = (void *)&(((struct sockaddr_in6*)sa)->sin6_addr);
		}
		else
		{
			inet_addr = NULL;
			result = EPFNOSUPPORT;  // Unsupported sa_family
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return inet_addr;
}


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
