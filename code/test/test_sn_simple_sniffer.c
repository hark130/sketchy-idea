/*
 *	Use skid_network to implement a basic packet sniffer.
 *
 *	Copy/paste one or more of the following...

sudo ./code/dist/test_sn_simple_sniffer.bin ICMP
sudo ./code/dist/test_sn_simple_sniffer.bin RAW
sudo ./code/dist/test_sn_simple_sniffer.bin TCP
sudo ./code/dist/test_sn_simple_sniffer.bin UDP

 *
 */

#define SKID_DEBUG					// Enable DEBUG logging

#include <arpa/inet.h>				// inet_ntoa()
#include <errno.h>					// EINVAL
#include <netinet/ip.h>				// struct iphdr
// #include <stdio.h>					 // fprintf()
#include <stdlib.h>					 // exit()
// #include <sys/socket.h>				 // AF_INET
#include <unistd.h>					// sleep()
#include "skid_debug.h"				// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_memory.h"			// alloc_skid_mem(), free_skid_mem()
#include "skid_network.h"			// call_recvfrom()


#define SOCKET_DOMAIN AF_INET 	 	 // Socket domain
#define SOCKET_TYPE SOCK_RAW		 // Socket type
#define RAW_ALIAS "RAW"              // Use this alias to check for a raw socket protocol


/*
 *	Description:
 *		Print the packet details.
 *
 *	Note:  The inet_ntoa() function converts the Internet host address
 *		to a string in IPv4 dotted-decimal notation.  The string is returned in a statically
 *		allocated buffer, which subsequent calls will overwrite.
 *
 *	Args:
 *		packet: Data read from socket.
 */
void print_packet(struct iphdr *packet);

/*
 *	Description:
 *		Print the usage.
 *
 *	Args:
 *		prog_name: argv[0].
 *		unsupported_alias: [Optional] If defined, tell the user this alias is unsupported.
 */
void print_usage(const char *prog_name, const char *unsupported_alias);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;                // Store errno and/or results here
	int sock_domain = SOCKET_DOMAIN;  // Socket domain
	int sock_type = SOCKET_TYPE;      // Socket type
	int sock_protocol = IPPROTO_RAW;  // Socket protocol
	int socket_fd = SKID_BAD_FD;      // Socket file descriptor
	struct iphdr *ip_packet = NULL;   // IP Header
	ssize_t packet_size = 0;          // Packet size, in bytes, as reported by recvfrom()
	void *raw_packet = NULL;          // Heap-allocated memory
	int flags = 0;                    // recvfrom() flags
	size_t buff_size = 65536;         // Size of the raw_packet buffer

	// INPUT VALIDATION
	if (argc != 2)
	{
		print_usage(argv[0], NULL);
		exit_code = EINVAL;
	}
	else if (!strcmp(argv[1], RAW_ALIAS))
	{
		sock_protocol = IPPROTO_RAW;
	}
	else
	{
		sock_protocol = resolve_alias(argv[1], &exit_code);
		if (sock_protocol < 0)
		{
			PRINT_ERROR(The call to resolve_alias() failed);
			PRINT_ERRNO(exit_code);
		}
		else if (sock_protocol != IPPROTO_ICMP && sock_protocol != IPPROTO_RAW \
				 && sock_protocol != IPPROTO_TCP && sock_protocol != IPPROTO_UDP)
		{
			print_usage(argv[0], argv[1]);
			exit_code = ENOPROTOOPT;  // This protocol is not supported
		}
	}
	
	// SETUP
	// Allocate memory
	if (ENOERR == exit_code)
	{
		raw_packet = alloc_skid_mem(buff_size, 1, &exit_code);
	}
	// Open the socket
	if (ENOERR == exit_code)
	{
		socket_fd = open_socket(sock_domain, sock_type, sock_protocol, &exit_code);
		if (SKID_BAD_FD == socket_fd)
		{
			// Failed to open the socket with this domain, type, and protocol
			PRINT_ERROR(The call to open_socket() failed);
			PRINT_ERRNO(exit_code);
			FPRINTF_ERR("%s - The call was open_socket(%d, %d, %d, %p)\n", DEBUG_ERROR_STR,
						sock_domain, sock_type, sock_protocol, &exit_code);  // DEBUGGING
			if (EPERM == exit_code)
			{
				fprintf(stderr, "%s.\nDid you neglect to elevate your privileges?\n"
					    "Consider the CAP_NET_RAW capability or sudo.\n", strerror(exit_code));
			}
		}
		else
		{
			printf("Starting to sniff around for %s packets...\n", argv[1]);
		}
	}

	// SNIFF
	while (ENOERR == exit_code)
	{
		// Read from the socket
		packet_size = call_recvfrom(socket_fd, flags, NULL, 0, raw_packet, buff_size, &exit_code);

		// Parse it
		if (packet_size > 0 && ENOERR == exit_code)
		{
			ip_packet = (struct iphdr *)raw_packet;
			print_packet(ip_packet);
		}
		else if (EAGAIN == exit_code || EWOULDBLOCK == exit_code)
		{
			exit_code = ENOERR;  // Nothing to see here
			sleep(1);  // Avoid CPU thrash with a tasteful sleep
			continue;  // Keep waiting for data			
		}
		else
		{
			FPRINTF_ERR("%s - call_recvfrom() read %ld bytes\n", DEBUG_INFO_STR, packet_size);
			PRINT_ERROR(The call to call_recvfrom() failed);
			PRINT_ERRNO(exit_code);
		}
	}

	// CLEANUP
	close_socket(&socket_fd, true);  // Best effort
	free_skid_mem(&raw_packet);  // Best effort

	// DONE
	exit(exit_code);
}


void print_packet(struct iphdr *packet)
{
	// LOCAL VARIABLES
	int results = 0;                 // Store errno values
	struct sockaddr_in src_address;  // Source information
	struct sockaddr_in dst_address;  // Destination information
	char src_str[16] = { 0 };        // IPv4 source address in dotted decimal
	char dst_str[16] = { 0 };        // IPv4 source address in dotted decimal
	uint16_t id_num = 0;             // IP header ID number
	uint8_t proto_num = 0;           // IP header layer 4 protocol number
	char *proto_alias = NULL;        // IP header layer 4 protocol alias

	// INPUT VALIDATION
	if (packet)
	{
		// SETUP
		memset(&src_address, 0x0, sizeof(src_address));
		memset(&dst_address, 0x0, sizeof(dst_address));
		src_address.sin_addr.s_addr = packet->saddr;
		dst_address.sin_addr.s_addr = packet->daddr;
		id_num = ntohs(packet->id);
		proto_num = packet->protocol;
		proto_alias = resolve_protocol(proto_num, &results);
		if (ENOERR != results)
		{
			PRINT_ERROR(The call to resolve_protocol() failed);
			PRINT_ERRNO(results);
		}
		// TRANSLATE
		strncpy(src_str, (char *)inet_ntoa(src_address.sin_addr), 15);
		strncpy(dst_str, (char *)inet_ntoa(dst_address.sin_addr), 15);

		// PRINT
		printf("FM: %15s\tTO: %15s\tSZ: %5d\tID: 0x%04x (%05d)\tTTL: %3u\tPROTO: %s (%d)\n",
			   src_str, dst_str, ntohs(packet->tot_len), id_num, id_num,
			   packet->ttl, proto_alias, proto_num);

		// CLEANUP
		free_skid_mem((void **)&proto_alias);  // Best effort
	}
	else
	{
		PRINT_ERROR(Received a NULL packet pointer);
	}
}


void print_usage(const char *prog_name, const char *unsupported_alias)
{
	if (unsupported_alias)
	{
		fprintf(stderr, "The protocol alias '%s' is not supported.\n\n", unsupported_alias);
	}
	fprintf(stderr, "Usage: %s <PROTOCOL: [ICMP/RAW/TCP/UDP]>\nReceiving of all IP protocols "
			"via IPPROTO_RAW is not possible using raw sockets.  See: raw(7).\n", prog_name);
}
