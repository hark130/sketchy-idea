/*
 *	Use skid_network to implement a packet sniffer for common protocols.
 *
 *	Copy/paste the following...

sudo ./code/dist/test_sn_multi_sniffer.bin

 *
 */

#define SKID_DEBUG					// Enable DEBUG logging

#include <arpa/inet.h>				// inet_ntoa()
#include <errno.h>					// EINVAL
#include <netinet/ip.h>				// struct iphdr
#include <stdlib.h>					// exit()
#include <unistd.h>					// sleep()
#include "skid_debug.h"				// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_memory.h"			// alloc_skid_mem(), free_skid_mem()
#include "skid_network.h"			// call_recvfrom()


#define SOCKET_DOMAIN AF_INET 	 	 // Socket domain
#define SOCKET_TYPE SOCK_RAW		 // Socket type
#define NUM_PROTO_NUMS 256			 // Size of the protocol-related arrays
#define INV_PROTOCOL (int)-1		 // Use this to indicate an invalid protocol number


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
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;                // Store errno and/or results here
	int sock_domain = SOCKET_DOMAIN;  // Socket domain
	int sock_type = SOCKET_TYPE;      // Socket type
	struct iphdr *ip_packet = NULL;   // IP Header
	ssize_t packet_size = 0;          // Packet size, in bytes, as reported by recvfrom()
	void *raw_packet = NULL;          // Heap-allocated memory
	int flags = MSG_DONTWAIT;         // recvfrom() flags
	size_t buff_size = 65536;         // Size of the raw_packet buffer
	char *temp_alias = NULL;          // Temp var to store protocol numbers resolved to their alias
	// Array of supported protocols terminated by an invalid protocol number
	int protocol_arr[NUM_PROTO_NUMS] = { IPPROTO_ICMP, IPPROTO_RAW, IPPROTO_TCP, IPPROTO_UDP,
	                                     INV_PROTOCOL };
	// Array of open socket fds (associated with protocol_arr indices)
	int socket_fd_arr[NUM_PROTO_NUMS] = { SKID_BAD_FD };

	// INPUT VALIDATION
	if (argc != 1)
	{
		print_usage(argv[0]);
		exit_code = EINVAL;
	}
	
	// SETUP
	// Allocate memory
	if (ENOERR == exit_code)
	{
		raw_packet = alloc_skid_mem(buff_size, 1, &exit_code);
	}
	// Initialize the array of open socket file descriptors
	if (ENOERR == exit_code)
	{
		for (int i = 0; i < NUM_PROTO_NUMS; i++)
		{
			socket_fd_arr[i] = SKID_BAD_FD;
		}
	}
	// Open the sockets
	if (ENOERR == exit_code)
	{
		for (int i = 0; i < NUM_PROTO_NUMS; i++)
		{
			if (INV_PROTOCOL != protocol_arr[i])
			{
				temp_alias = resolve_protocol(protocol_arr[i], &exit_code);
				if (ENOERR != exit_code)
				{
					PRINT_ERROR(The call to resolve_protocol() failed);
					FPRINTF_ERR("%s - The call was resolve_protocol(%d, %p) on index %d\n",
						        DEBUG_ERROR_STR, protocol_arr[i], &exit_code, i);
					PRINT_ERRNO(exit_code);
					break;
				}
				else
				{
					printf("Preparing to sniff %s packets...\n", temp_alias);
					free_skid_mem((void **)&temp_alias);  // Best effort
					socket_fd_arr[i] = open_socket(sock_domain, sock_type, protocol_arr[i],
						                           &exit_code);
					if (ENOERR != exit_code || SKID_BAD_FD == socket_fd_arr[i])
					{
						PRINT_ERROR(The call to open_socket() failed);
						PRINT_ERRNO(exit_code);
						if (EPERM == exit_code)
						{
							fprintf(stderr, "%s.\nDid you neglect to elevate your privileges?\n"
								    "Consider the CAP_NET_RAW capability or sudo.\n",
								    strerror(exit_code));
						}
						break;
					}
				}
			}
			else
			{
				break;  // Found the array termination
			}
		}
	}

	// SNIFF
	while (ENOERR == exit_code)
	{
		for (int i = 0; i < NUM_PROTO_NUMS; i++)
		{
			if (SKID_BAD_FD == socket_fd_arr[i])
			{
				break;  // Found the array termination, so start over
			}
			// Read from the socket
			packet_size = call_recvfrom(socket_fd_arr[i], flags, NULL, 0, raw_packet, buff_size,
				                        &exit_code);

			// Parse it
			if (packet_size > 0 && ENOERR == exit_code)
			{
				ip_packet = (struct iphdr *)raw_packet;
				print_packet(ip_packet);
				memset(raw_packet, 0x0, packet_size);
			}
			else if (EAGAIN == exit_code || EWOULDBLOCK == exit_code)
			{
				exit_code = ENOERR;  // Nothing to see here
				// sleep(1);  // Avoid CPU thrash with a tasteful sleep
				continue;  // Keep waiting for data			
			}
			else
			{
				FPRINTF_ERR("%s - call_recvfrom() read %ld bytes\n", DEBUG_INFO_STR, packet_size);
				PRINT_ERROR(The call to call_recvfrom() failed);
				PRINT_ERRNO(exit_code);
				break;
			}
		}
	}

	// CLEANUP
	for (int i = 0; i < NUM_PROTO_NUMS; i++)
	{ 
		close_socket(&(socket_fd_arr[i]), true);  // Best effort
	}
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


void print_usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s\n", prog_name);
}
