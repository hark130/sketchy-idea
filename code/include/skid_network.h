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

/*
 *	Description:
 *		"Assign a name to a socket" using a struct to specify the address.
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
 *		Marks a socket as a passive socket so it may accept incoming connection requests.
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
 *		Open a socket.
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
 *		On success, a file descriptor for the new socket is returned.  On error, -1 is returned,
 *		and errnum is set appropriately.
 */
int open_socket(int domain, int type, int protocol, int *errnum);

#endif  /* __SKID_NETWORK__ */
