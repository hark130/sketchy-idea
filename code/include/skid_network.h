#ifndef __SKID_NETWORK__
#define __SKID_NETWORK__

/*
 *	TYPICAL USAGE:
 *	1. Easy button
 *
 *	2. Manual steps
 *		- int sockfd = open_socket(AF_INET, SOCK_STREAM, 0)
 *		- bind_struct(sockfd, address_struct, sizeof(address_struct))
 *		- listen_socket(sockfd, 10)
 *		- accept(sockfd)/read(sockfd)/write(sockfd)/etc.
 */

#include <netdb.h>			// struct addrinfo
#include <stdbool.h>    	// bool, false, true
#include <sys/socket.h>		// socklen_t

#define SKID_BAD_FD (signed int)-1  // Use this to standardize "invalid" file descriptors

/*
 *	Description:
 *		Accept an incoming connection request to a listening socket.
 *
 *	Notes:
 *		Extracts the first connection request on the queue of pending connections for the
 *		listening socket, sockfd, creates a new connected socket, and returns a new
 *		file descriptor referring to that socket.  The newly created socket is not in the
 *		listening state.  The original socket sockfd is unaffected by this call.
 *
 *	Args:
 *		sockfd: A file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.
 *		addr: [Optional/Out] This structure is filled in with the address of the peer socket,
 *			as known to the communications layer.  The exact format of the address returned addr
 *			is determined by the socket's address family (see socket(2) and the respective
 *			protocol man pages).  When addr is NULL, nothing is filled in; in this case,
 *			addrlen is not used, and should also be NULL.
 *		addrlen: [Optional/In/Out] The size of addr on the way in and the way out.  the caller must
 *			initialize it to contain the size (in bytes) of the structure pointed to by addr.
 *			On return it will contain the actual size of the peer address.  If addr is NULL,
 *			this value should also be NULL.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		On success, a file descriptor for the accepted socket (a non-negative integer).
 *		On error, SKID_BAD_FD is returned, errnum is set appropriately, and addrlen is left
 *		unchanged.
 */
int accept_client(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int *errnum);

/*
 *	Description:
 *		"Assign a name to a socket" using a struct to specify the address.  This function will not
 *		close sockfd.
 *
 *	Args:
 *		sockfd: The socket file descriptor to assign an address to.
 *		addr: A pointer to the address family-specific struct defining the address to bind to.
 *			For AF_INET, see ip(7); for AF_INET6, see ipv6(7); for AF_UNIX, see unix(7), etc.
 *		addrlen: Specifies the size, in bytes, of addr.
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int bind_struct(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/*
 *	Description:
 *		Close a socket file descriptor and sets it to SKID_BAD_FD (if it was successfully closed).
 *
 *	Args:
 *		sockfd: [In/Out] A pointer to a socket file descriptor to close.
 *		quiet: If true, silences all logging/debugging.
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int close_socket(int *sockfd, bool quiet);

/*
 *	Description:
 *		Convert socket address storage struct addr to a human-readable IP address.
 *
 *	Args:
 *		addr: Struct pointer to conver to an IP address.
 *		ip_buff: [Out] Buffer to store the converted IP address.  Must be at least INET6_ADDRSTRLEN
 *			bytes long.
 *		ip_size: The size, in bytes, of ip_buff.
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int convert_sas_ip(struct sockaddr_storage *addr, char *ip_buff, size_t ip_size);

/*
 *	Description:
 *		Use freeaddrinfo() to free the linked list created by get_addr_info() and set it to NULL.
 *
 *	Args:
 *		res: [In/Out] A pointer to the head node of the addrinfo struct pointers provided
 *			by get_addr_info().
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int free_addr_info(struct addrinfo **res);

/*
 *	Description:
 *		Populate addrinfo structures for use with bind(2) or connect(2).  Calls getaddrinfo()
 *		under the hood.  The caller is responsible for freeing the contents of res by calling
 *		free_addr_info() (or freeaddrinfo()).
 *
 *	Notes:
 *		The getaddrinfo() function allocates and initializes a linked list of addrinfo structures,
 *		one for each network address that matches node and service, subject to any restrictions
 *		imposed by hints, and returns a pointer to the start of the list in res.  The items
 *		in the linked list are linked by the ai_next field.
 *
 *	Args:
 *		node: An indication of the host (e.g., hostname, IP address).  Either node or service,
 *			but not both, may be NULL.
 *		service: Sets the port in each returned address structure.  Could be NULL, a port number,
 *			or a serivce name (see: services(5)).  Either node or service, but not both,
 *			may be NULL.
 *		hints: [Optional] A pointer to a addrinfo structure that specifies criteria for selecting
 *			the socket address structures returned in the list pointed to by res.
 *			See getaddrinfo(3) for details on how to setup the structure.  If hints is NULL,
 *			it is equivalent to is equivalent to setting ai_socktype and ai_protocol to 0;
 *			ai_family to AF_UNSPEC; and ai_flags to (AI_V4MAPPED | AI_ADDRCONFIG).
 *		res: [Out] A linked list of addrinfo structures allocated and initialized by
 *			getaddrinfo().  It is the caller's responsibility to free this linked list by
 *			calling free_addr_info() (or freeaddrinfo()).
 *
 *	Returns:
 *		On success, zero is returned.  On error, -1 is returned for an unspecified getaddrinfo()
 *		errcode, or errno is returned.
 */
int get_addr_info(const char *node, const char *service, const struct addrinfo *hints,
	              struct addrinfo **res);

/*
 *	Description:
 *		Marks a socket as a passive socket so it may accept incoming connection requests.  This
 *		function will not close sockfd.
 *
 *	Args:
 *		sockfd: A file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.
 *		backlog: Defines the maximum length to which the queue of pending connections for
 *			the socket may grow.  If a connection request arrives when the queue is full,
 *			the client may receive an error with an indication of ECONNREFUSED or, if the
 *			underlying protocol supports retransmission, the request may be ignored so that a
 *			later reattempt at connection succeeds.
 *
 *	Returns:
 *		On success, zero is returned.  On error, errno is returned.
 */
int listen_socket(int sockfd, int backlog);

/*
 *	Description:
 *		TO DO: DON'T DO NOW... CONTINUE HERE(?) FOR THE MANUAL TEST IMPLEMENTATION?
 */
// int open_named_socket();

/*
 *	Description:
 *		Open a socket.  It is the caller's responsibility to close the socket file descriptor
 *		returned by this function.
 *
 *	Args:
 *		domain: Specifies a communication domain; this selects the protocol family which will be
 *			used for communication.  E.g., AF_INET, AF_UNIX.  These families are defined
 *			in <sys/socket.h>.
 *		type: Specifies communication semantics.  E.g., SOCK_STREAM, SOCK_DGRAM, SOCK_RAW.
 *			Since Linux 2.6.27, the type argument serves a second purpose: in addition to
 *			specifying a socket type, it may include the bitwise OR of any of the following
 *			values, to modify the behavior of socket():  SOCK_NONBLOCK, SOCK_CLOEXEC.
 *		protocol: Specifies a particular protocol to be used with the socket.  Normally only a
 *			single protocol exists to support a particular socket type within a given protocol
 *			family, in which case protocol can be specified as 0.  However, it is possible that
 *			many protocols may exist, in which case a particular protocol must be specified in
 *			this manner.
 *		errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *	Returns:
 *		On success, a file descriptor for the new socket is returned.  On error, SKID_BAD_FD is
 *		returned, and errnum is set appropriately.
 */
int open_socket(int domain, int type, int protocol, int *errnum);

#endif  /* __SKID_NETWORK__ */
