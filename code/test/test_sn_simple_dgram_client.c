/*
 *	Manually test skid_network's manual socket, bind, and listen functions.
 *	This binary use the function wrappers to replicate this behavior:
 *		https://github.com/beejjorgensen/bgnet/blob/main/src/bgnet_part_0600_clientserver.md
 *
 *	Copy/paste the following...

# All values are hard-coded so no arguments are necessary
./code/dist/test_sn_simple_dgram_client.bin

 *
 */

#define SKID_DEBUG					// Enable DEBUG logging

#include <arpa/inet.h>				// inet_addr()
#include <errno.h>					// EINVAL
#include <netinet/in.h>				// inet_addr()
#include <stdio.h>					// fprintf()
#include <stdlib.h>					// exit()
#include <sys/socket.h>				// AF_INET
#include <unistd.h>					// fork()
#include "skid_debug.h"				// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_network.h"


#define SERVER_SLEEP 1  			// Number of seconds the server sleeps while awiting connection
#define SERVER_DOMAIN AF_INET 	 	// Server socket domain
#define SERVER_TYPE SOCK_DGRAM		// Server socket type
#define SERVER_PROTOCOL IPPROTO_UDP	// Server socket protocol
#define SERVER_PORT 5678			// The port clients will connect to


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;                           // Store errno and/or results here
	// int result = 0;                              // Additional errno values
	int server_domain = SERVER_DOMAIN;           // Server socket domain
	int server_type = SERVER_TYPE;               // Server socket type
	unsigned short server_port = SERVER_PORT;    // Port the server is listening on
	int socket_fd = SKID_BAD_FD;                 // Server file descriptor
	const char *node = "127.0.0.1";              // Hostname/IP of the server
	// struct addrinfo hints;                       // Selection criteria
	// struct addrinfo *servinfo = NULL;            // Out argument for get_addr_info()
	// struct addrinfo *temp_serv = NULL;           // Use this to walk the servinfo linked list
	char message[] = { "Hello, world!" };        // Message for the client to send to the server
	int sendto_flags = 0;                        // See sendto(2)
	struct sockaddr_in servaddr;

	// INPUT VALIDATION
	if (argc == 2)
	{
		node = argv[1];  // User has specified a hostname/IP to contact the server
	}
	else if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s [Optional server name/IP]\n", argv[0]);
	   exit_code = EINVAL;
	}

	// SETUP
	if (!exit_code)
	{
		// Zeroize the struct
		memset(&servaddr, 0x0, sizeof(servaddr));
		servaddr.sin_family = server_domain;
		servaddr.sin_addr.s_addr = inet_addr(node);
		servaddr.sin_port = htons(server_port);
	}

	// CONNECT
	// Open Socket
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
	}

	// Connect
	if (!exit_code)
	{
		exit_code = connect_socket(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if (exit_code)
		{
			PRINT_ERROR(The call to connect_socket() failed);
			PRINT_ERRNO(exit_code);
		}
	}

	// Send
	if (!exit_code)
	{
		FPRINTF_ERR("%s - Client: attempting to send data...\n", DEBUG_INFO_STR);
        exit_code = send_to_socket(socket_fd, message, sendto_flags, (struct sockaddr *)NULL, 0);
        if (!exit_code)
        {
			FPRINTF_ERR("%s - Client: message sent!\n", DEBUG_INFO_STR);
        }
        else
        {
			PRINT_ERROR(The call to send_to_socket() failed);
			PRINT_ERRNO(exit_code);
        }
	}

	// CLEANUP
	close_socket(&socket_fd, true);  // Best effort

	// DONE
	exit(exit_code);
}
