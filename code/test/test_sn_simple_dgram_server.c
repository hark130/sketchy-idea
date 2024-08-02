/*
 *	Manually test skid_network's manual socket, bind, and listen functions.
 *	This binary use the function wrappers to replicate this behavior:
 *		https://github.com/beejjorgensen/bgnet/blob/main/src/bgnet_part_0600_clientserver.md
 *
 *	Copy/paste the following...

# All values are hard-coded so no arguments are necessary
./code/dist/test_sn_simple_dgram_server.bin <SERVER_IP_ADDR>

 *
 */

#define SKID_DEBUG			// Enable DEBUG logging

#include <arpa/inet.h>				// inet_addr()
#include <errno.h>					// EINVAL
#include <netinet/in.h>				// inet_addr()
#include <stdio.h>					// fprintf()
#include <stdlib.h>					// exit()
#include <sys/socket.h>				// AF_INET
#include <unistd.h>					// fork()
#include "skid_debug.h"				// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_memory.h"			// free_skid_mem()
#include "skid_network.h"
#include "skid_signals.h"			// handle_all_children(), set_signal_handler()


#define SERVER_SLEEP 1  			 // Number of seconds the server sleeps while awiting connection
#define SERVER_DOMAIN AF_INET 		 // Server socket domain
#define SERVER_TYPE SOCK_DGRAM		 // Server socket type
#define SERVER_PROTOCOL IPPROTO_UDP  // Server socket protocol
#define SERVER_PORT 5678			 // The port clients will connect to


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;                             // Store errno and/or results here
	int server_domain = SERVER_DOMAIN;             // Server socket domain
	int server_type = SERVER_TYPE;                 // Server socket type
	int server_protocol = SERVER_PROTOCOL;         // Server socket protocol
	int socket_fd = SKID_BAD_FD;                   // Server file descriptor
	const char *node = NULL;                       // Hostname/IP to use for the server
	char client_addr[INET6_ADDRSTRLEN+1] = { 0 };  // Converted socket address to human readable IP
	char *client_msg = NULL;                       // Message read from the client file descriptor
	int recv_flags = 0;                            // See recv(2)
	char *protocol_name = NULL;                    // Official name of the server_protocol
	struct sockaddr_in servaddr;			       // Server address
	struct sockaddr_in cliaddr;					   // Client address
	socklen_t cliaddr_size = sizeof(cliaddr);      // The size of cliaddr

	// INPUT VALIDATION
	if (argc == 2)
	{
		node = argv[1];  // User has specified a node to use
	}
	else if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s [Optional hostname/IP]\n", argv[0]);
	   exit_code = EINVAL;
	}

	// SETUP
	// Setup server struct
	if (!exit_code)
	{
		// Zeroize the server struct
		memset(&servaddr, 0x0, sizeof(servaddr));
		if (node)
		{
			servaddr.sin_addr.s_addr = inet_addr(node);
		}
		else
		{
			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		servaddr.sin_port = htons(SERVER_PORT);
	    servaddr.sin_family = server_domain;  // AKA domain
	}
	// Setup client struct
	if (!exit_code)
	{
		memset(&cliaddr, 0x0, sizeof(cliaddr));  // Zeroize
	}
	// "Name the socket"
	if (!exit_code)
	{
		// Open the socket
		socket_fd = open_socket(server_domain, server_type, 0, &exit_code);
		if (SKID_BAD_FD == socket_fd)
		{
			// Failed to open the socket with this domain, type, and protocol
			PRINT_ERROR(The call to open_socket() failed);
			PRINT_ERRNO(exit_code);
			FPRINTF_ERR("%s - The call was open_socket(%d, %d, %d, %p)\n", DEBUG_ERROR_STR,
				        server_domain, server_type, 0, &exit_code);
		}
		else
		{
			exit_code = bind_struct(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
			if (exit_code)
			{
				// Failed to "name the socket"
				PRINT_ERROR(The call to bind_struct() failed);
				PRINT_ERRNO(exit_code);
				FPRINTF_ERR("%s - The call was bind_struct(%d, %p, %lu)\n", DEBUG_ERROR_STR,
					        socket_fd, &servaddr, sizeof(servaddr));
				close_socket(&socket_fd, false);  // This socket is no good
			}
		}
	}
	// Receive data
	if (!exit_code)
	{
		FPRINTF_ERR("%s - Setting the server socket to wait for a client\n", DEBUG_INFO_STR);
		do
		{
			// Block until data received from a client
			if (!exit_code)
			{
				if (SERVER_PROTOCOL == server_protocol)
				{
					client_msg = recv_from_socket(socket_fd, recv_flags,
						                          (struct sockaddr *)&cliaddr,
												  &cliaddr_size, &exit_code);
					if (exit_code)
					{
						PRINT_ERROR(The call to recv_from_socket() failed);
						PRINT_ERRNO(exit_code);
					}
				}
				else
				{
					protocol_name = resolve_protocol(server_protocol, &exit_code);  // Best effort
					FPRINTF_ERR("%s - Skipping recv from an ineligible client protocol [%d]: %s\n",
						        DEBUG_INFO_STR, server_protocol, protocol_name);
					exit_code = EPROTONOSUPPORT;
				}
			}
			// Output
			if (!exit_code)
			{
				exit_code = convert_sas_ip((struct sockaddr_storage *)&cliaddr, client_addr,
					                       INET6_ADDRSTRLEN * sizeof(char));
				if (!exit_code)
				{
					printf("%s - Server: received message from %s: %s\n",
						   DEBUG_INFO_STR, client_addr, client_msg);
				}
				else
				{
					PRINT_ERROR(The call to convert_sas_ip() failed);
					PRINT_ERRNO(exit_code);
				}
			}
		} while (ENOERR == exit_code);
	}

	// CLEANUP
	free_skid_mem((void **)&protocol_name);  // Best effort
	free_skid_mem((void **)&client_msg);  // Best effort
	close_socket(&socket_fd, true);  // Best effort

	// DONE
	exit(exit_code);
}
