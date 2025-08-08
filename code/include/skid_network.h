#ifndef __SKID_NETWORK__
#define __SKID_NETWORK__

/*
 *  TYPICAL USAGE:
 *      int sockfd = open_socket(AF_INET, SOCK_STREAM, 0)
 *      bind_struct(sockfd, address_struct, sizeof(address_struct))
 *      listen_socket(sockfd, 10)
 *      accept(sockfd)/read(sockfd)/write(sockfd)/etc.
 */

#include <netdb.h>                          // struct addrinfo
#include <stdbool.h>                        // bool, false, true
#include <sys/socket.h>                     // socklen_t
#include "skid_macros.h"                    // SKID_BAD_FD

/*
 *  Description:
 *      Accept an incoming connection request to a listening socket.
 *
 *  Notes:
 *      Extracts the first connection request on the queue of pending connections for the
 *      listening socket, sockfd, creates a new connected socket, and returns a new
 *      file descriptor referring to that socket.  The newly created socket is not in the
 *      listening state.  The original socket sockfd is unaffected by this call.
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.
 *      addr: [Optional/Out] This structure is filled in with the address of the peer socket,
 *          as known to the communications layer.  The exact format of the address returned addr
 *          is determined by the socket's address family (see socket(2) and the respective
 *          protocol man pages).  When addr is NULL, nothing is filled in; in this case,
 *          addrlen is not used, and should also be NULL.
 *      addrlen: [Optional/In/Out] The size of addr on the way in and the way out.  the caller must
 *          initialize it to contain the size (in bytes) of the structure pointed to by addr.
 *          On return it will contain the actual size of the peer address.  If addr is NULL,
 *          this value should also be NULL.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      On success, a file descriptor for the accepted socket (a non-negative integer).
 *      On error, SKID_BAD_FD is returned, errnum is set appropriately, and addrlen is left
 *      unchanged.
 */
int accept_client(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int *errnum);

/*
 *  Description:
 *      "Assign a name to a socket" using a struct to specify the address.  This function will not
 *      close sockfd.
 *
 *  Args:
 *      sockfd: The socket file descriptor to assign an address to.
 *      addr: A pointer to the address family-specific struct defining the address to bind to.
 *          For AF_INET, see ip(7); for AF_INET6, see ipv6(7); for AF_UNIX, see unix(7), etc.
 *      addrlen: Specifies the size, in bytes, of addr.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int bind_struct(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/*
 *  Description:
 *      Standardize the call to recvfrom() and response to its return values.
 *
 *  Args:
 *      sockfd: Socket file descriptor to recv from.
 *      flags: A bit-wise OR of zero or more flags (see: recvfrom(2)).  Passed directly to
 *          recvfrom() without validation.
 *      src_addr: [Optional/Out] Passed directly to recvfrom() without validation.
 *      addrlen: [Optional/Out] Passed directly to recvfrom() without validation.
 *      buff: [Out] Pointer to buffer to read into.
 *      buff_size: The size of buf in bytes.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      The number of bytes read by recvfrom() on success, -1 on failure (sets errno value in
 *      errnum).
 */
ssize_t call_recvfrom(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
                      char *buff, size_t buff_size, int *errnum);

/*
 *  Description:
 *      Close a socket file descriptor and sets it to SKID_BAD_FD (if it was successfully closed).
 *
 *  Args:
 *      sockfd: [In/Out] A pointer to a socket file descriptor to close.
 *      quiet: If true, silences all logging/debugging.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int close_socket(int *sockfd, bool quiet);

/*
 *  Description:
 *      Connects the sockfd to the address specified by addr.
 *
 *  Notes:
 *      The format of the address in addr is determined by the address space of the socket sockfd;
 *      see socket(2) for further details.  If the socket sockfd is of type SOCK_DGRAM,
 *      then addr is the address to which datagrams are sent by default, and the only address
 *      from which datagrams are received.  If the socket is of type SOCK_STREAM or SOCK_SEQPACKET,
 *      this call attempts to make a connection to the socket that is bound to the address
 *      specified by addr.
 *
 *  Args:
 *      sockfd: File descriptor of a server socket.
 *      addr: Address of the socket.
 *      addrlen: The size of addr.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int connect_socket(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/*
 *  Description:
 *      Convert socket address storage struct addr to a human-readable IP address.
 *
 *  Args:
 *      addr: Struct pointer to conver to an IP address.
 *      ip_buff: [Out] Buffer to store the converted IP address.  Must be at least INET6_ADDRSTRLEN
 *          bytes long.
 *      ip_size: The size, in bytes, of ip_buff.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int convert_sas_ip(struct sockaddr_storage *addr, char *ip_buff, size_t ip_size);

/*
 *  Description:
 *      Use freeaddrinfo() to free the linked list created by get_addr_info() and set it to NULL.
 *
 *  Args:
 *      res: [In/Out] A pointer to the head node of the addrinfo struct pointers provided
 *          by get_addr_info().
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int free_addr_info(struct addrinfo **res);

/*
 *  Description:
 *      Populate addrinfo structures for use with bind(2) or connect(2).  Calls getaddrinfo()
 *      under the hood.  The caller is responsible for freeing the contents of res by calling
 *      free_addr_info() (or freeaddrinfo()).
 *
 *  Notes:
 *      The getaddrinfo() function allocates and initializes a linked list of addrinfo structures,
 *      one for each network address that matches node and service, subject to any restrictions
 *      imposed by hints, and returns a pointer to the start of the list in res.  The items
 *      in the linked list are linked by the ai_next field.
 *
 *  Args:
 *      node: An indication of the host (e.g., hostname, IP address).  Either node or service,
 *          but not both, may be NULL.
 *      service: Sets the port in each returned address structure.  Could be NULL, a port number,
 *          or a serivce name (see: services(5)).  Either node or service, but not both,
 *          may be NULL.
 *      hints: [Optional] A pointer to a addrinfo structure that specifies criteria for selecting
 *          the socket address structures returned in the list pointed to by res.
 *          See getaddrinfo(3) for details on how to setup the structure.  If hints is NULL,
 *          it is equivalent to is equivalent to setting ai_socktype and ai_protocol to 0;
 *          ai_family to AF_UNSPEC; and ai_flags to (AI_V4MAPPED | AI_ADDRCONFIG).
 *      res: [Out] A linked list of addrinfo structures allocated and initialized by
 *          getaddrinfo().  It is the caller's responsibility to free this linked list by
 *          calling free_addr_info() (or freeaddrinfo()).
 *
 *  Returns:
 *      On success, zero is returned.  On error, -1 is returned for an unspecified getaddrinfo()
 *      errcode, or errno is returned.
 */
int get_addr_info(const char *node, const char *service, const struct addrinfo *hints,
                  struct addrinfo **res);

/*
 *  Description:
 *      Marks a socket as a passive socket so it may accept incoming connection requests.  This
 *      function will not close sockfd.
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.
 *      backlog: Defines the maximum length to which the queue of pending connections for
 *          the socket may grow.  If a connection request arrives when the queue is full,
 *          the client may receive an error with an indication of ECONNREFUSED or, if the
 *          underlying protocol supports retransmission, the request may be ignored so that a
 *          later reattempt at connection succeeds.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.
 */
int listen_socket(int sockfd, int backlog);

/*
 *  Description:
 *      Use getsockopt() to get the send buffer size information at the socket API level.
 *      See: getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, ...) for more information.
 *
 *  Args:
 *      sockfd: Socket file descriptor to fetch information about.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      On success, the size of sockfd's send buffer.
 *      On failure, -1 and errnum is set appropriately.  ENOBUFS is used to indicate a socket
 *          that reported a send buffer size of zero (0).
 */
int get_socket_opt_sndbuf(int sockfd, int *errnum);

/*
 *  Description:
 *      Open a socket.  It is the caller's responsibility to close the socket file descriptor
 *      returned by this function.
 *
 *  Args:
 *      domain: Specifies a communication domain; this selects the protocol family which will be
 *          used for communication.  E.g., AF_INET, AF_UNIX.  These families are defined
 *          in <sys/socket.h>.
 *      type: Specifies communication semantics.  E.g., SOCK_STREAM, SOCK_DGRAM, SOCK_RAW.
 *          Since Linux 2.6.27, the type argument serves a second purpose: in addition to
 *          specifying a socket type, it may include the bitwise OR of any of the following
 *          values, to modify the behavior of socket():  SOCK_NONBLOCK, SOCK_CLOEXEC.
 *      protocol: Specifies a particular protocol to be used with the socket.  Normally only a
 *          single protocol exists to support a particular socket type within a given protocol
 *          family, in which case protocol can be specified as 0.  However, it is possible that
 *          many protocols may exist, in which case a particular protocol must be specified in
 *          this manner.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      On success, a file descriptor for the new socket is returned.  On error, SKID_BAD_FD is
 *      returned, and errnum is set appropriately.
 */
int open_socket(int domain, int type, int protocol, int *errnum);

/*
 *  Description:
 *      Dynamically read a message from a socket based on the given protocol.
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a named socket to receive from.
 *      flags: [Optional?] A bit-wise OR of zero or more flags given to the underlying system call.
 *          This value will be ignored for non-applicable function calls (e.g., read_fd()).
 *      protocol: The struct addrinfo.ai_protocol the socket was bound to.
 *          This function currently supports: SOCK_STREAM, SOCK_DGRAM, SOCK_DCCP.
 *          See socket(2) for details.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      Pointer to the heap-allocated buffer, on success.  NULL on error and errnum is set with
 *      details.  EPROTONOSUPPORT is used to indicate an unsupported protocol argument value.
 */
char *receive_socket(int sockfd, int flags, int protocol, int *errnum);

/*
 *  Description:
 *      Read a message from a socket, using recv(), into a heap-allocated array.
 *      It is the caller's responsibility to free the buffer with free_skid_mem().
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a socket to receive from.
 *      flags: A bit-wise OR of zero or more flags, as defined in recv(2):
 *          MSG_CMSG_CLOEXEC, MSG_DONTWAIT, MSG_ERRQUEUE, MSG_OOB, MSG_PEEK, MSG_TRUNC, MSG_WAITALL.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      Pointer to the heap-allocated buffer, on success.  NULL on error (check errnum for details).
 */
char *recv_socket(int sockfd, int flags, int *errnum);

/*
 *  Description:
 *      Read a message from a socket, using recvfrom(), into a heap-allocated array.
 *      It is the caller's responsibility to free the buffer with free_skid_mem().
 *
 *  Note:
 *      If src_addr is not NULL, and the underlying protocol provides the source address of the
 *      message, that source address is placed in the buffer pointed to by src_addr.
 *      In this case, addrlen is a value-result argument.  Before the call, it should be
 *      initialized to the size of the buffer associated with src_addr.  Upon return, addrlen
 *      is updated to contain the actual size of the source address.  The returned address is
 *      truncated if the buffer provided is too small; in this case, addrlen will return a value
 *      greater than was supplied to the call.
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a socket to receive from.
 *      flags: A bit-wise OR of zero or more flags, as defined in recv(2):
 *          MSG_CMSG_CLOEXEC, MSG_DONTWAIT, MSG_ERRQUEUE, MSG_OOB, MSG_PEEK, MSG_TRUNC, MSG_WAITALL.
 *      src_addr: [Optional/Out] A pointer to the storage location for the source address of the
 *          incoming connection.  Not validated.  Passed directly to recvfrom().
 *      addrlen: [Optional/Out] A pointer to the storage location for the actual size of the
 *          source address.  Not validated.  Passed directly to recvfrom().
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      Pointer to the heap-allocated buffer, on success.  NULL on error (check errnum for details).
 *      ENODATA is used to indicate there was no data to receive.
 */
char *recv_from_socket(int sockfd, int flags, struct sockaddr *src_addr, socklen_t *addrlen,
                       int *errnum);

/*
 *  Description:
 *      Resolve a protocol alias into its protocol number by searching the protocols database
 *      (see: protocols(5)).
 *
 *  Args:
 *      proto_alias: The alias to search for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      The matching protocol number on success.  -1 on failure and errnum is set appropriately.
 *      Uses ENOPROTOOPT to indicate the protocol alias wasn't found in the database.
 */
int resolve_alias(const char *proto_alias, int *errnum);

/*
 *  Description:
 *      Use getprotobynumber() to resolve a protocol number into its official name and copy that
 *      into a heap-allocated string.  It is the caller's responsibility to free the buffer
 *      with free_skid_mem().
 *
 *  Args:
 *      protocol: A protocol number expected to be found in the protocols database.
 *          See: protocols(5).
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      Pointer to the heap-allocated buffer, on success.  NULL on error (check errnum for details).
 *      EPROTO is used to indicate an unresolved protocol number.  The IPPROTO_RAW protocol number
 *      is resolved as SKID_RAW_SOCK_ALIAS (see: skid_macros.h).
 */
char *resolve_protocol(int protocol, int *errnum);

/*
 *  Description:
 *      Send a message on a socket file descriptor using send().
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a socket to send to.
 *      msg: The nul-terminated message to write to sockfd.
 *      flags: A bit-wise OR of zero or more flags, as defined in send(2):
 *          MSG_CONFIRM, MSG_DONTROUTE, MSG_DONTWAIT, MSG_EOR, MSG_MORE, MSG_NOSIGNAL, MSG_OOB.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.  ENOTCONN is returned when
 *      sockfd was not actually connected.
 */
int send_socket(int sockfd, const char *msg, int flags);

/*
 *  Description:
 *      Send a messsage on a socket file descriptor using sendto().
 *
 *  Note:
 *      If this function is used on a connection-mode (SOCK_STREAM, SOCK_SEQPACKET) socket, the
 *      arguments dest_addr and addrlen are ignored (and should be NULL and 0).  Otherwise, the
 *      address of the target is given by dest_addr with addrlen specifying its size.
 *      For connection-mode sockets, consider using send_socket().
 *
 *  Args:
 *      sockfd: A file descriptor that refers to a socket to send to.
 *      msg: The nul-terminated message to write to sockfd.
 *      flags: A bit-wise OR of zero or more flags, as defined in sendto(2):
 *          MSG_CONFIRM, MSG_DONTROUTE, MSG_DONTWAIT, MSG_EOR, MSG_MORE, MSG_NOSIGNAL, MSG_OOB.
 *      dest_addr: [Optional] A pointer to the storage location for the destination address of
 *          the message.  Not validated.  Passed directly to sendto().
 *      addrlen: [Optional] The actual size of the dest_addr (destination address) argument.
 *          Not validated.  Passed directly to sendto().
 *      chunk_it: If true, avoid EMSGSIZE errors by chunking the msg to fit in sockfd's
 *          send buffer.  The default chunk size is SKID_CHUNK_SIZE (see: skid_macros.h).
 *          If false, sendto() msg as-is.
 *
 *  Returns:
 *      On success, zero is returned.  On error, errno is returned.  EISCONN may be returned if
 *      sockfd is a connection-based socket and dest_addr/addrlen were defined.  ENOTCONN is
 *      returned when sockfd was not actually connected.
 */
int send_to_socket(int sockfd, const char *msg, int flags, const struct sockaddr *dest_addr,
                   socklen_t addrlen, bool chunk_it);

#endif  /* __SKID_NETWORK__ */
