/*
 *	This library defines functionality to help manager server/clients.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_file_descriptors.h"		// close_fd()
#include "skid_debug.h"					// PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"				// ENOERR
#include "skid_memory.h"				// alloc_skid_mem()
#include "skid_network.h"				// SKID_BAD_FD
#include "skid_validation.h"			// validate_skid_err(), validate_skid_sockfd()
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

#define SKID_NET_BUFF_SIZE 1024  // Starting buffer size to read into


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *	Description:
 *		Check for an existing buffer pointer.  If one does not exist, make the first allocation.
 *
 *	Args:
 *		output_buf: [Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *			a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *			output_size will be updated.
 *		output_size: [Out] Pointer to the size of output_buf.
 *
 *	Returns:
 *		0 on success, errno on failure.
 */
int check_sn_pre_alloc(char **output_buf, size_t *output_size);

/*
 *	Description:
 *		Determine if the bytes_read can fit into the output buffer based on its size and current
 *		length.
 *
 *	Args:
 *		bytes_read: The number of bytes to append to the output buffer.
 *		output_len: The number of bytes currently in the buffer.
 *		output_size: Total size of the output buffer.
 *
 *	Returns:
 *		True if there's room.  False if there isn't (time to reallocate), on for invalid args.
 */
bool check_sn_space(size_t bytes_read, size_t output_len, size_t output_size);

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
 *	Description:
 *		Reallocate a buffer: allocate a new buffer double the *output_size, copy the contents of
 *		*output_buf into the new buffer, free the old buffer, update the Out arguments.
 *		It is the caller's responsibility to free the buffer with free_skid_mem().
 *
 *	Args:
 *		output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *			a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *			output_size will be updated.
 *		output_size: [In/Out] Pointer to the size of output_buf.
 *
 *	Returns:
 *		0 on success, errno on failure.  EOVERFLOW is used to indicate the output_size can not
 *		be doubled without overflowing the size_t data type.
 */
int realloc_sock_dynamic(char **output_buf, size_t *output_size);

/*
 *	Description:
 *		Read the contents of the file descriptor into a heap-allocated buffer.  If the buffer ever
 *		fills then this function will (effectively) reallocate more space.  It will read until
 *		no other data can be read or an error occurred.  It is the caller's responsibility to
 *		free the buffer with free_skid_mem().
 *
 *	Args:
 *		sockfd: Socket file descriptor to recv from.
 *		flags: A bit-wise OR of zero or more flags (see: recvfrom(2)).
 *		src_addr: [Optional/Out] Passed directly to recvfrom() without validation.
 *		addrlen: [Optional/Out] Passed directly to recvfrom() without validation.
 *		output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *			a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *			output_size will be updated.
 *		output_size: [In/Out] Pointer to the size of output_buf.
 *
 *	Returns:
 *		0 on success, errno on failure.
 */
int recv_from_socket_dynamic(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
	                         char **output_buf, size_t *output_size);

/*
 *	Description:
 *		Read the contents of the file descriptor into a heap-allocated buffer.  If the buffer ever
 *		fills then this function will (effectively) reallocate more space.  It will read until
 *		no other data can be read or an error occurred.  It is the caller's responsibility to
 *		free the buffer with free_skid_mem().
 *
 *	Args:
 *		sockfd: Socket file descriptor to recv from.
 *		flags: A bit-wise OR of zero or more flags (see: recv(2), recv_socket()).
 *		output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *			a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *			output_size will be updated.
 *		output_size: [In/Out] Pointer to the size of output_buf.
 *
 *	Returns:
 *		0 on success, errno on failure.
 */
int recv_socket_dynamic(int sockfd, int flags, char **output_buf, size_t *output_size);

/*
 *	Description:
 *		Validate common In/Out args on behalf of the library.
 *
 *	Args:
 *		output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *			a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *			output_size will be updated.
 *		output_size: [In/Out] Pointer to the size of output_buf.
 *
 *	Returns:
 *		0 on success, errno on failed validation.
 */
int validate_sn_args(char **output_buf, size_t *output_size);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int accept_client(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;          // Errno values
	int client_fd = SKID_BAD_FD;  // Accepted client file descriptor

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);
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
	result = validate_skid_sockfd(sockfd);

	// BIND IT
	if (ENOERR == result)
	{
		if (bind(sockfd, addr, addrlen))
		{
			result = errno;
			PRINT_ERROR(The call to bind() failed);
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
	// Handled by close_fd()

	// CLOSE IT
	result = close_fd(sockfd, quiet);

	// DONE
	return result;
}


int connect_socket(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno values

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);

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
	result = validate_skid_sockfd(sockfd);

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
	result = validate_skid_err(errnum);

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
			result = validate_skid_sockfd(sockfd);
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


char *receive_socket(int sockfd, int flags, int protocol, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_skid_err(errnum);  // Errno values
	char *msg = NULL;                        // Return value from underlying SKID library call

	// INPUT VALIDATION
	// protocol
	if (ENOERR == result)
	{
		// Underlying SKID library call will validate remaining input as appropriate
		if (SOCK_STREAM == protocol)
		{
			// read()
			msg = read_fd(sockfd, &result);
		}
		else if (SOCK_DGRAM == protocol || SOCK_DCCP == protocol)
		{
			// recv()
			msg = recv_socket(sockfd, flags, &result);
		}
		else
		{
			// Don't forget to update the comment block w/ any additional protocols supported
			result = EPROTONOSUPPORT;  // Unsupported protocol
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return msg;
}


char *recv_socket(int sockfd, int flags, int *errnum)
{
	// LOCAL VARIABLES
	char *msg = NULL;                            // Heap-allocated copy of the msg read from sockfd
	size_t msg_len = 0;                          // The length of msg (after it's recv()'d)
	int result = ENOERR;                         // Errno values

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);
	if (ENOERR == result)
	{
		result = validate_skid_err(errnum);
	}

	// RECEIVE IT
	if (ENOERR == result)
	{
		result = recv_socket_dynamic(sockfd, flags, &msg, &msg_len);
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return msg;
}


char *recv_from_socket(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
	                   int *errnum)
{
	// LOCAL VARIABLES
	char *msg = NULL;                            // Heap-allocated copy of the msg read from sockfd
	size_t msg_len = 0;                          // The length of msg (after it's recv()'d)
	int result = ENOERR;                         // Errno values

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);
	if (ENOERR == result)
	{
		result = validate_skid_err(errnum);
	}

	// RECV FROM IT
	if (ENOERR == result)
	{
		result = recv_from_socket_dynamic(sockfd, flags, src_addr, addrlen, &msg, &msg_len);
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return msg;
}


char *resolve_protocol(int protocol, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;                     // Errno values
	struct protoent *protocol_entry = NULL;  // Return value from getprotobynumber()
	bool opened = false;                     // Track whether the database may have been opened
	char *official_name = NULL;              // Protocol's official name in heap memory

	// INPUT VALIDATION
	result = validate_skid_err(errnum);

	// RESOLVE IT
	// Get the protoent struct
	if (ENOERR == result)
	{
		protocol_entry = getprotobynumber(protocol);
		opened = true;
		if (NULL == protocol_entry)
		{
			result = EPROTO;  // Unresolved protocol
		}
	}
	// Copy the official name
	if (ENOERR == result)
	{
		official_name = copy_skid_string(protocol_entry->p_name, &result);
	}

	// CLEANUP
	if (true == opened)
	{
		endprotoent();  // Closes the connection to the protocols database
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return official_name;
}


int send_socket(int sockfd, const char *msg, int flags)
{
	// LOCAL VARIABLES
	size_t msg_len = 0;      // Length of msg
	ssize_t bytes_sent = 0;  // Return value from send()
	int result = ENOERR;     // Errno values

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);
	if (ENOERR == result)
	{
		result = validate_skid_string(msg, false);  // Can not be empty
	}

	// SEND IT
	if (ENOERR == result)
	{
		msg_len = strlen(msg);
		bytes_sent = send(sockfd, (void *)msg, msg_len * sizeof(char), flags);
		if (bytes_sent < 0)
		{
			result = errno;
			PRINT_ERROR(The call to send() failed);
			PRINT_ERRNO(result);
		}
		else if (bytes_sent < (msg_len * sizeof(char)))
		{
			PRINT_WARNG(The call to send() only finished a partial send);
			result = send_socket(sockfd, msg + bytes_sent, flags);  // Finish sending or force error
		}
	}

	// DONE
	return result;
}


int send_to_socket(int sockfd, const char *msg, int flags, const struct sockaddr *dest_addr,
	               socklen_t addrlen)
{
	// LOCAL VARIABLES
	size_t msg_len = 0;      // Length of msg
	ssize_t bytes_sent = 0;  // Return value from send()
	int result = ENOERR;     // Errno values

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);
	if (ENOERR == result)
	{
		result = validate_skid_string(msg, false);  // Can not be empty
	}

	// SEND TO IT
	if (ENOERR == result)
	{
		msg_len = strlen(msg);
		bytes_sent = sendto(sockfd, msg, msg_len, flags, dest_addr, addrlen);
		if (bytes_sent < 0)
		{
			result = errno;
			PRINT_ERROR(The call to sendto() failed);
			PRINT_ERRNO(result);
		}
		else if (bytes_sent < (msg_len * sizeof(char)))
		{
			PRINT_WARNG(The call to sendto() only finished a partial send);
			// Finish sending or force error
			result = send_to_socket(sockfd, msg + bytes_sent, flags, dest_addr, addrlen);
		}
	}

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int check_sn_pre_alloc(char **output_buf, size_t *output_size)
{
	// LOCAL VARIABLES
	int result = ENOERR;   // Success of execution
	char *tmp_ptr = NULL;  // Return value from allocation

	// INPUT VALIDATION
	result = validate_sn_args(output_buf, output_size);

	// CHECK IT
	if (ENOERR == result)
	{
		if (NULL == *output_buf)
		{
			tmp_ptr = alloc_skid_mem(SKID_NET_BUFF_SIZE, sizeof(char), &result);
			if (NULL == tmp_ptr)
			{
				PRINT_ERROR(The call to alloc_skid_mem() failed);
				PRINT_ERRNO(result);
			}
			else
			{
				*output_buf = tmp_ptr;  // Store the pointer
				*output_size = SKID_NET_BUFF_SIZE;  // Update the size
			}
		}
	}

	// DONE
	return result;
}


bool check_sn_space(size_t bytes_read, size_t output_len, size_t output_size)
{
	// LOCAL VARIABLES
	bool has_room = false;  // Is there enough room in the output buffer to store bytes_read

	// CHECK IT
	if (bytes_read > 0 && output_size > 0)
	{
		if ((output_size - output_len) >= bytes_read)
		{
			has_room = true;
		}
	}

	// DONE
	return has_room;
}


void *get_inet_addr(struct sockaddr *sa, int *errnum)
{
	// LOCAL VARIABLES
	void *inet_addr = NULL;  // Pointer to the relevant struct in*_addr member
	int result = ENOERR;     // Errno values

	// INPUT VALIDATION
	result = ( NULL == sa ) ? EINVAL : validate_skid_err(errnum);

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


int realloc_sock_dynamic(char **output_buf, size_t *output_size)
{
	// LOCAL VARIABLES
	int result = ENOERR;   // Success of execution
	size_t new_size = 0;   // New size of the allocation
	char *tmp_ptr = NULL;  // Return value from allocation

	// INPUT VALIDATION
	result = validate_sn_args(output_buf, output_size);
	if (ENOERR == result)
	{
		if (0 >= *output_size)
		{
			result = EINVAL;  // They should have called check_sn_pre_alloc()
		}
	}

	// REALLOCATE
	// Determine new size
	if (ENOERR == result)
	{
		// Check for maximum
		if (SKID_MAX_SZ == *output_size)
		{
			result = EOVERFLOW;  // Buffer size is already at its maximum value
		}
		// Check for overflow
		else if (*output_size > (SKID_MAX_SZ - *output_size))
		{
			result = EOVERFLOW;  // Not enough room left to double it
		}
		else
		{
			new_size = 2 * (*output_size);
		}
	}
	// Allocate
	if (ENOERR == result)
	{
		tmp_ptr = alloc_skid_mem(new_size, sizeof(char), &result);
		if (NULL == tmp_ptr)
		{
			PRINT_ERROR(The call to alloc_skid_mem() failed);
			PRINT_ERRNO(result);
		}
	}
	// Copy old into new
	if (ENOERR == result)
	{
		strncpy(tmp_ptr, *output_buf, *output_size);
	}
	// Free old
	if (ENOERR == result)
	{
		result = free_skid_mem((void **)output_buf);
		*output_size = 0;  // Zero out the size
	}
	// Update out arguments
	if (ENOERR == result)
	{
		*output_buf = tmp_ptr;  // Store the pointer
		*output_size = new_size;  // Update the size
	}

	// DONE
	return result;
}


int recv_from_socket_dynamic(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
	                         char **output_buf, size_t *output_size)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);       // Success of execution
	char local_buf[SKID_NET_BUFF_SIZE] = { 0 };  // Local buffer
	ssize_t num_read = 0;                        // Number of bytes read
	size_t output_len = 0;                       // The length of *output_buf's string
	char *tmp_ptr = NULL;                        // Temp pointer

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		result = validate_sn_args(output_buf, output_size);
	}

	// SETUP
	if (ENOERR == result)
	{
		result = check_sn_pre_alloc(output_buf, output_size);
	}

	// RECVFROM DYNAMIC
	if (ENOERR == result)
	{
		while (1)
		{
			output_len = strlen(*output_buf);  // Get the current length of output_buf
			// Read into local buff
			num_read = recvfrom(sockfd, local_buf, sizeof(local_buf), flags, src_addr, addrlen);
			if (num_read < 0)
			{
				result = errno;
				PRINT_ERROR(The call to recvfrom() failed);
				PRINT_ERRNO(result);
				break;  // Error
			}
			else if (0 == num_read)
			{
				 FPRINTF_ERR("%s - Call to recvfrom() reached EOF\n", DEBUG_INFO_STR);
				 break;  // Done reading
			}
			// Check for room
			if (false == check_sn_space(num_read, output_len, *output_size))
			{
				// Not enough room?  Reallocate.
				result = realloc_sock_dynamic(output_buf, output_size);
				if (ENOERR != result)
				{
					PRINT_ERROR(The call to realloc_sock_dynamic() failed);
					PRINT_ERRNO(result);
					break;  // Stop on error
				}
			}
			// Copy local buff into *output_buf
			if (true == check_sn_space(num_read, output_len, *output_size))
			{
				strncat(*output_buf, local_buf, *output_size - output_len);  // Add local to output
				memset(local_buf, 0x0, sizeof(local_buf));  // Zeroize the local buffer
			}
			else
			{
				PRINT_ERROR(Logic Failure - realloc_sock_dynamic succeeded incorrectly);
				result = EOVERFLOW;
			}
		}
	}

	// VERIFY NUL-TERMINATED
	if (ENOERR == result)
	{
		if (output_len == *output_size)
		{
			tmp_ptr = copy_skid_string(*output_buf, &result);
			if (ENOERR == result)
			{
				free_skid_mem((void **)output_buf);  // Free the old buffer
				*output_buf = tmp_ptr;  // Save the new buffer
				*output_size += 1;  // Made room for nul-termination
			}
		}
	}

	// CLEANUP
	if (ENOERR != result)
	{
		// output_buf
		free_skid_mem((void **)output_buf);
		// output_size
		if (output_size)
		{
			*output_size = 0;
		}
		// tmp_ptr
		free_skid_mem((void **)&tmp_ptr);
	}

	// DONE
	return result;
}


int recv_socket_dynamic(int sockfd, int flags, char **output_buf, size_t *output_size)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);       // Success of execution
	char local_buf[SKID_NET_BUFF_SIZE] = { 0 };  // Local buffer
	ssize_t num_read = 0;                        // Number of bytes read
	size_t output_len = 0;                       // The length of *output_buf's string
	char *tmp_ptr = NULL;                        // Temp pointer

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		result = validate_sn_args(output_buf, output_size);
	}

	// SETUP
	if (ENOERR == result)
	{
		result = check_sn_pre_alloc(output_buf, output_size);
	}

	// READ DYNAMIC
	if (ENOERR == result)
	{
		while (1)
		{
			output_len = strlen(*output_buf);  // Get the current length of output_buf
			// Read into local buff
			num_read = recv(sockfd, local_buf, sizeof(local_buf), flags);
			if (0 == num_read)
			{
				 FPRINTF_ERR("%s - Call to recv() reached EOF\n", DEBUG_INFO_STR);
				 break;  // Done reading
			}
			// Check for room
			if (false == check_sn_space(num_read, output_len, *output_size))
			{
				// Not enough room?  Reallocate.
				result = realloc_sock_dynamic(output_buf, output_size);
				if (ENOERR != result)
				{
					PRINT_ERROR(The call to realloc_sock_dynamic() failed);
					PRINT_ERRNO(result);
					break;  // Stop on error
				}
			}
			// Copy local buff into *output_buf
			if (true == check_sn_space(num_read, output_len, *output_size))
			{
				strncat(*output_buf, local_buf, *output_size - output_len);  // Add local to output
				memset(local_buf, 0x0, sizeof(local_buf));  // Zeroize the local buffer
			}
			else
			{
				PRINT_ERROR(Logic Failure - realloc_sock_dynamic succeeded incorrectly);
				result = EOVERFLOW;
			}
		}
	}

	// VERIFY NUL-TERMINATED
	if (ENOERR == result)
	{
		if (output_len == *output_size)
		{
			tmp_ptr = copy_skid_string(*output_buf, &result);
			if (ENOERR == result)
			{
				free_skid_mem((void **)output_buf);  // Free the old buffer
				*output_buf = tmp_ptr;  // Save the new buffer
				*output_size += 1;  // Made room for nul-termination
			}
		}
	}

	// CLEANUP
	if (ENOERR != result)
	{
		// output_buf
		free_skid_mem((void **)output_buf);
		// output_size
		if (output_size)
		{
			*output_size = 0;
		}
		// tmp_ptr
		free_skid_mem((void **)&tmp_ptr);
	}

	// DONE
	return result;
}


int validate_sn_args(char **output_buf, size_t *output_size)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Validation result

	// VALIDATE IT
	// output_buf
	if (NULL == output_buf)
	{
		result = EINVAL;  // NULL pointer
	}
	// output_size
	else if (NULL == output_size)
	{
		result = EINVAL;  // NULL pointer
	}
	// output_buf && output_size
	else
	{
		if (NULL != *output_buf && *output_size <= 0)
		{
			result = EINVAL;  // Buffer pointer exists but size is invalid
			PRINT_ERROR(Invalid size of an existing buffer pointer);
		}
	}

	// DONE
	return result;
}
