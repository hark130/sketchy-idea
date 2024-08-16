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
 *		Use getsockname() to determine the socket family associated with sockfd.
 *
 *	Args:
 *		sockfd: Socket file descriptor to fetch information about.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		The struct sockaddr.sa_family value on success, 0 on failure (sets errno value in errnum).
 */
sa_family_t get_socket_family(int sockfd, int *errnum);

/*
 *	Description:
 *		A "lite" wrapper around getsockopt().  Not for external use as the necessary option_value
 *		size isn't standard.
 *
 *	Notes:
 *		Most socket-level options utilize an int argument for option_value.
 *		For Boolean options, a zero value indicates that the option is disabled and a non-zero
 *			value indicates that the option is enabled.
 *		If the size of the option value is greater than option_len, the value stored in the
 *			object pointed to by the option_value argument shall be silently truncated.
 *
 *	Args:
 *		sockfd: Socket file descriptor to fetch information about.
 *		level: The appropriate level identifier for the protocol controlling the option.
 *			Common examples include: SOL_SOCKET (socket API level), IPPROTO_TCP (TCP),
 *			and IPPROTO_UDP (UDP).  See: The <netinet/in.h> header, getsockopt(2), getprotoent(3),
 *			and protocols(5) for more information.
 *		option_name: The singled option to be retrieved.  The include file <sys/socket.h> contains
 *			definitions for socket level options.  Common examples include: SO_ERROR (socket error
 *			status), SO_SNDBUF (send buffer size), SO_TYPE (socket type).
 *		option_value: [Optional/Out] Option value of sockfd at the specified protocol level.
 *			If no option value is to be supplied or returned, option_value may be NULL.
 *		option_len: [Optional/In] The size of option_value.
 *
 *	Returns:
 *		0 on success, errno on failure.
 */
int get_socket_option(int sockfd, int level, int option_name,
					  void *restrict option_value, socklen_t *restrict option_len);

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
 *		Determine the size of the data waiting to be read from sockfd.  Calls recvfrom() with
 *		the MSG_PEEK, MSG_TRUNC, and MSG_DONTWAIT flags.
 *
 *	Notes:
 *		This will not work for unix(7) sockets (e.g., AF_UNIX, AF_LOCAL) (see: recv(2)).
 *		Datagram sockets in various domains permit zero-length datagrams.  When such a datagram
 *			is received, the return value is 0.
 *
 *	Args:
 *		sockfd: Socket file descriptor to recv from.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		The real length of the packet or datagram.  Some "error" situations will be treated as
 *		"nothing to read" and 0 will be returned: zero-length datagrams, no data to read, etc.
 *		On an actual error, -1 is returned but the errno value will be stored in errnum.
 */
ssize_t recv_from_size(int sockfd, int *errnum);

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
 *		A "lite" wrapper around the module's call to sendto(), standardizing error response.
 *		This function does not validate input.  It does, however, attempt to recursively
 *		complete partial sends (bytes successfully sent are less than len).
 *
 *	Args:
 *		sockfd: Specifies the socket file descriptor.
 *		buf: Points to a buffer containing the message to be sent.
 *		len: Specifies the size of the message in bytes.
 *		flags: Specifies the type of message transmission.
 *		dest_addr: Points to a sockaddr structure containing the destination address.
 *			The length and format of the address depend on the address family of the socket.
 *		addrlen: Specifies the length of the sockaddr structure pointed to by the dest_addr arg.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		Upon successful completion, send_to() shall return the number of bytes sent.
 *		Partial sends, number of bytes sent < len, are treated as successful.
 *		Otherwise, -1 shall be returned and errnum set to indicate the error.
 */
ssize_t send_to(int sockfd, const void *buf, size_t len, int flags,
				const struct sockaddr *dest_addr, socklen_t addrlen, int *errnum);

/*
 *	Description:
 *		Chunks buf into get_socket_opt_sndbuf() segments and passes them to send_to().
 *		This function barely validates input: non-NULL buf and valid len.
 *
 *	Args:
 *		sockfd: Specifies the socket file descriptor.
 *		buf: Points to a buffer containing the message to be sent.
 *		len: Specifies the size of the message in bytes.
 *		flags: Specifies the type of message transmission.
 *		dest_addr: Points to a sockaddr structure containing the destination address.
 *			The length and format of the address depend on the address family of the socket.
 *		addrlen: Specifies the length of the sockaddr structure pointed to by the dest_addr arg.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		Upon successful completion, send_to() shall return the number of bytes sent.
 *		Partial sends, number of bytes sent < len, are treated as successful.
 *		Otherwise, -1 shall be returned and errnum set to indicate the error.
 */
ssize_t send_to_chunk(int sockfd, const void *buf, size_t len, int flags,
					  const struct sockaddr *dest_addr, socklen_t addrlen, int *errnum);

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


ssize_t call_recvfrom(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
					  char *buff, size_t buff_size, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);       // Success of execution
	ssize_t num_read = -1;                       // Number of bytes read on success, -1 for error
	static bool mentioned_block = false;         // Only DEBUG blocking results once
	bool print = true;                           // Dynamically avoid copy/pasted statements

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		if (NULL == errnum || NULL == buff || buff_size <= 0)
		{
			result = EINVAL;  // Invalid input
		}
	}

	// RECEIVE FROM IT
	if (ENOERR == result)
	{
		num_read = recvfrom(sockfd, buff, buff_size, flags, src_addr, addrlen);
		if (num_read < 0)
		{
			result = errno;
			if (MSG_DONTWAIT == (MSG_DONTWAIT & flags) \
				&& (EAGAIN == result || EWOULDBLOCK == result))
			{
				if (false == mentioned_block)
				{
					mentioned_block = true;  // Avoid verbose "waiting" messages
				}
				else
				{
					print = false;  // Skip mentioning it
				}
			}
			if (true == print)
			{
				PRINT_ERROR(The call to recvfrom() failed);
				PRINT_ERRNO(result);
			}
		}
		else if (0 == num_read)
		{
			 FPRINTF_ERR("%s - Call to recvfrom() reached EOF\n", DEBUG_INFO_STR);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return num_read;
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


int get_socket_opt_sndbuf(int sockfd, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_skid_err(errnum);  // Errno values
	int optval = -1;                         // Option value
	socklen_t optlen = sizeof(optval);       // Size of option value

	// GET SNDBUF
	if (ENOERR == result)
	{
		result = get_socket_option(sockfd, SOL_SOCKET, SO_SNDBUF, &optval, &optlen);
		if (ENOERR != result)
		{
			PRINT_ERROR(The call to get_socket_option() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return optval;
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
	char *msg = NULL;     // Heap-allocated copy of the msg read from sockfd
	size_t msg_len = 0;   // The length of msg (after it's recv()'d)
	int result = ENOERR;  // Errno values

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
	char *msg = NULL;       // Heap-allocated copy of the msg read from sockfd
	ssize_t data_size = 0;  // Size of the data waiting in sockfd
	int result = ENOERR;    // Errno values

	// INPUT VALIDATION
	result = validate_skid_sockfd(sockfd);
	if (ENOERR == result)
	{
		result = validate_skid_err(errnum);
	}

	// RECEIVE IT
	// Size it
	if (ENOERR == result)
	{
		data_size = recv_from_size(sockfd, &result);
		if (data_size < 0)
		{
			PRINT_ERROR(The call to recv_from_size() failed);
			PRINT_ERRNO(result);
		}
		else if (0 == data_size)
		{
			result = ENODATA;  // No data to receive
		}
	}
	// Allocate
	if (ENOERR == result)
	{
		msg = alloc_skid_mem(data_size + 1, 1, &result);
	}
	// Read
	if (ENOERR == result)
	{
		if (data_size != call_recvfrom(sockfd, flags, src_addr, addrlen, msg, data_size, &result))
		{
			PRINT_ERROR(The call to call_recvfrom() failed);
			PRINT_ERRNO(result);
		}
	}

	// CLEANUP
	if (ENOERR != result)
	{
		free_skid_mem((void **)&msg);  // Best effort
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return msg;
}


int resolve_alias(const char *proto_alias, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;                   // Store errno values
	struct protoent *protocol_ptr = NULL;  // Protocol database entry
	int protocol_num = -1;                 // Resolved protocol number

	// INPUT VALIDATION
	if (NULL == errnum || NULL == proto_alias || !(*proto_alias))
	{
		result = EINVAL;  // Bad input
	}

	// RESOLVE IT
	if (ENOERR == result)
	{
		result = ENOPROTOOPT;  // Default result, post-validation
		while (protocol_num < 0)
		{
			errno = 0;  // Clear errno
			protocol_ptr = getprotoent();
			if (NULL == protocol_ptr)
			{
				break;
			}
			for (int i = 0; NULL != protocol_ptr->p_aliases[i]; i++)
			{
				if (!strcmp(proto_alias, protocol_ptr->p_aliases[i]))
				{
					protocol_num = protocol_ptr->p_proto;  // Found it!
					result = ENOERR;
					break;
				}
			}
		}

		// CLEANUP
		endprotoent();  // Close the database connection
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return protocol_num;
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
				   socklen_t addrlen, bool chunk_it)
{
	// LOCAL VARIABLES
	size_t msg_size = 0;     // Size of msg
	ssize_t bytes_sent = 0;  // Return value from send()
	int result = ENOERR;     // Errno values

	// INPUT VALIDATION
	// sockfd
	result = validate_skid_sockfd(sockfd);
	// msg
	if (ENOERR == result)
	{
		result = validate_skid_string(msg, false);  // Can not be empty
	}
	// Message size
	if (ENOERR == result)
	{
		msg_size = strlen(msg) * sizeof(char);
		if (msg_size > SKID_MAX_DGRAM_DATA_IPV4 && false == chunk_it)
		{
			FPRINTF_ERR("%s - A message size of %lu exceeds known limits and is expected to fail "
						"with a [%d] '%s' error (without chunking enabled).\n",
						DEBUG_WARNG_STR, msg_size, EMSGSIZE, strerror(EMSGSIZE));
		}
	}

	// SEND TO IT
	if (ENOERR == result)
	{
		// Chunk it?
		if (SKID_CHUNK_SIZE < msg_size && true == chunk_it)
		{
			bytes_sent = send_to_chunk(sockfd, msg, msg_size, flags, dest_addr, addrlen, &result);
		}
		else
		{
			bytes_sent = send_to(sockfd, msg, msg_size, flags, dest_addr, addrlen, &result);
		}
		// Validate bytes sent
		if (bytes_sent < 0)
		{
			PRINT_ERROR(The call to send_to() failed);
			PRINT_ERRNO(result);
		}
		else if (bytes_sent < msg_size)
		{
			PRINT_WARNG(The call to send_to() only succeeded in a partial send);
		}
		else if (bytes_sent == msg_size)
		{
			FPRINTF_ERR("%s - The call to send_to() perfectly sent %lu bytes.\n",
						DEBUG_INFO_STR, bytes_sent);
		}
		else
		{
			PRINT_ERROR(The call to send_to() sent more bytes than expected);
			FPRINTF_ERR("%s - The call to send_to() sent %lu bytes instead of %lu.\n",
						DEBUG_ERROR_STR, bytes_sent, msg_size);
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
		// Leave an extra byte for the nul-terminator
		if ((output_size - output_len) >= (bytes_read + 1))
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


sa_family_t get_socket_family(int sockfd, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);         // Success of execution
	sa_family_t sock_fam = 0;                      // Socket family
	struct sockaddr sock_data;                     // Socket data struct
	socklen_t sock_data_size = sizeof(sock_data);  // Size of the sockaddr struct

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		result = validate_skid_err(errnum);
	}

	// SETUP
	if (ENOERR == result)
	{
		memset(&sock_data, 0x0, sock_data_size);  // Zeroize the struct
	}

	// GET FAMILY
	if (ENOERR == result)
	{
		if (getsockname(sockfd, &sock_data, &sock_data_size))
		{
			result = errno;
			PRINT_ERROR(The call to getsockname() failed);
			PRINT_ERRNO(result);
		}
		else
		{
			sock_fam = sock_data.sa_family;
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return sock_fam;
}


int get_socket_option(int sockfd, int level, int option_name,
					  void *restrict option_value, socklen_t *restrict option_len)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);   // Success of execution

	// GET IT
	if (ENOERR == result)
	{
		if (getsockopt(sockfd, level, option_name, option_value, option_len))
		{
			result = errno;
			PRINT_ERROR(The call to getsockopt() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
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


ssize_t recv_from_size(int sockfd, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);            // Success of execution
	ssize_t data_size = 0;                            // Size of sockfd's data
	sa_family_t sock_fam = 0;                         // Family associated with sockfd
	int flags = MSG_PEEK | MSG_TRUNC | MSG_DONTWAIT;  // Flags passed to recvfrom()
	char small_buff[1] = { 0 };                       // Very small buffer
	size_t small_size = sizeof(small_buff);           // Very small size

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		result = validate_skid_err(errnum);
	}
	if (ENOERR == result)
	{
		sock_fam = get_socket_family(sockfd, &result);
		if (0 == sock_fam)
		{
			PRINT_ERROR(The call to get_socket_family() failed);
			PRINT_ERRNO(result);
		}
		else if (AF_UNIX == sock_fam || AF_LOCAL == sock_fam)
		{
			result = EAFNOSUPPORT;  // MSG_TRUNC support not implemented for this domain/family
			PRINT_ERROR(The flags used here have not been implemented with this socket family);
		}
	}

	// SIZE IT
	if (ENOERR == result)
	{
		// data_size = recvfrom(sockfd, NULL, 0, flags);
		// Just get the size of the data in sockfd
		data_size = call_recvfrom(sockfd, flags, NULL, NULL, small_buff, small_size, &result);
		if (data_size < 0)
		{
			// Dynamically handle results
			if (EAGAIN == result || EWOULDBLOCK == result)
			{
				data_size = 0;  // Nothing to read
				result = ENOERR;  // Everything is fine.  Nothing to see here.
			}
			else if (result)
			{
				PRINT_ERROR(The call to recvfrom() failed);
				PRINT_ERRNO(result);
			}
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return data_size;
}


int recv_from_socket_dynamic(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
							 char **output_buf, size_t *output_size)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);       // Success of execution
	char local_buf[SKID_NET_BUFF_SIZE + 1] = { 0 };  // Local buffer
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
		output_len = strlen(*output_buf);  // Get the current length of output_buf
		// Read into local buff
		num_read = call_recvfrom(sockfd, flags, src_addr, addrlen, local_buf,
								 SKID_NET_BUFF_SIZE * sizeof(char), &result);
		if (num_read > 0)
		{
			// Check for room
			if (false == check_sn_space(num_read, output_len, *output_size))
			{
				// Not enough room?  Reallocate.
				result = realloc_sock_dynamic(output_buf, output_size);
				if (ENOERR != result)
				{
					PRINT_ERROR(The call to realloc_sock_dynamic() failed);
					PRINT_ERRNO(result);
				}
			}
			if (ENOERR == result)
			{
				// Copy local buff into *output_buf
				if (true == check_sn_space(num_read, output_len, *output_size))
				{
					// Add local to output
					strncat(*output_buf, local_buf, (*output_size) - output_len);
					memset(local_buf, 0x0, sizeof(local_buf));  // Zeroize the local buffer
				}
				else
				{
					PRINT_ERROR(Logic Failure - realloc_sock_dynamic succeeded incorrectly);
					result = EOVERFLOW;
				}
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


ssize_t send_to(int sockfd, const void *buf, size_t len, int flags,
				const struct sockaddr *dest_addr, socklen_t addrlen, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;     // Validation result
	ssize_t bytes_sent = 0;  // Number of bytes sent
	ssize_t more_bytes = 0;  // Additional send

	// SEND IT
	bytes_sent = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	if (bytes_sent < 0)
	{
		result = errno;
		PRINT_ERROR(The call to sendto() failed);
		PRINT_ERRNO(result);
	}
	else
	{
		// Handle partial sends
		while (bytes_sent < len)
		{
			PRINT_WARNG(The call to sendto() only finished a partial send);
			// Finish sending or force error
			more_bytes = send_to(sockfd, buf + bytes_sent, len - bytes_sent,
								 flags, dest_addr, addrlen, &result);
			if (more_bytes < 0)
			{
				PRINT_ERROR(The recursive call to send_to() failed to finish the partial send);
				PRINT_ERRNO(result);
				result = ENOERR;  // Inform the caller of the partial success
				break;  // Avoid the infinite loop
			}
			else
			{
				bytes_sent += more_bytes;  // We sent more so report more
			}
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return bytes_sent;
}


ssize_t send_to_chunk(int sockfd, const void *buf, size_t len, int flags,
					  const struct sockaddr *dest_addr, socklen_t addrlen, int *errnum)
{
	// LOCAL VARIABLES
	int result = validate_skid_fd(sockfd);  // Validation result
	ssize_t bytes_sent = 0;                 // Number of bytes sent
	int snd_buf_size = 0;                   // Size of sockfd's send buffer
	int chunk_size = 0;                     // Size of each chunk to send
	int num_runs = 0;                       // Number of runs to iterate through
	ssize_t tmp_sent = 0;                   // Temporary bytes sent for iterating send_to() calls
	size_t tmp_len = 0;                     // Temporary bytes to send for iterating send_to() calls

	// INPUT VALIDATION
	if (ENOERR == result)
	{
		if (NULL == buf || len <= 0)
		{
			result = EINVAL;  // Invalid input
		}
	}

	// SEND CHUNKS
	// Size the send buffer
	if (ENOERR == result)
	{
		// Get the size of the send buffer
		snd_buf_size = get_socket_opt_sndbuf(sockfd, &result);
		if (snd_buf_size < 0)
		{
			PRINT_ERROR(The call to get_socket_opt_sndbuf() failed);
			PRINT_ERRNO(result);
		}
		else if (0 == snd_buf_size)
		{
			PRINT_ERROR(The call to get_socket_opt_sndbuf() succeeded with a size of zero);
			result = ENOBUFS;  // No buffer space available
		}
		else
		{
			if (SKID_CHUNK_SIZE < snd_buf_size)
			{
				chunk_size = SKID_CHUNK_SIZE;
			}
			else
			{
				chunk_size = snd_buf_size;  // Send buffer is smaller so we have to use that value
			}
		}
	}
	// Chunk it?
	if (ENOERR == result)
	{
		if (chunk_size >= len)
		{
			bytes_sent = send_to(sockfd, buf, len, flags, dest_addr, addrlen, &result);
		}
		else
		{
			num_runs = len / chunk_size;
			if (len % chunk_size)
			{
				num_runs++;  // One last run for the remainder
			}
			for (int i = 0; i < num_runs; i++)
			{
				tmp_len = (i + 1 == num_runs) ? (len % chunk_size) : chunk_size;  // Last run?
				tmp_sent = send_to(sockfd, buf + (i * chunk_size), tmp_len, flags, dest_addr,
								   addrlen, &result);
				if (tmp_sent < 0)
				{
					PRINT_ERROR(The send_to_chunk() call to send_to() failed);
					PRINT_ERRNO(result);
					break;  // Error
				}
				else
				{
					bytes_sent += tmp_sent;  // Keep track of the number of bytes sent
					if (tmp_sent < tmp_len)
					{
						PRINT_ERROR(The send_to_chunk() send_to() call resulted in a partial send);
						break;  // Stop iterating because it throws off the algorithm
					}
				}
			}
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return bytes_sent;
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
