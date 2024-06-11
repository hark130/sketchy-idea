/*
 *	Manually test skid_network's manual socket, bind, and listen functions.
 *	This binary use the function wrappers to replicate this behavior:
 *		https://github.com/beejjorgensen/bgnet/blob/main/src/bgnet_part_0600_clientserver.md
 *
 *	Copy/paste the following...

# All values are hard-coded so no arguments are necessary
./code/dist/test_sn_simple_stream_server.bin

 *
 */

#define SKID_DEBUG			// Enable DEBUG logging

#include <errno.h>			// EINVAL
#include <stdio.h>			// fprintf()
#include <stdlib.h>			// exit()
#include "skid_debug.h"		// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()


#define SERVER_DOMAIN AF_INET 		// Server socket domain
#define SERVER_TYPE SOCK_STREAM		// Server socket type
#define SERVER_PROTOCOL 0 			// Server socket protocol
#define PORT "1234"					// The port clients will connect to
#define BACKLOG 10   				// Maximum number of pending connections to queue


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;                      // Store errno and/or results here
	int server_domain = SERVER_DOMAIN;      // Server socket domain
	int server_type = SERVER_TYPE;          // Server socket type
	int server_protocol = SERVER_PROTOCOL;  // Server socket protocol
	int server_fd = -1;                     // Server file descriptor

	// INPUT VALIDATION
	if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s\n", argv[0]);
	   exit_code = EINVAL;
	}

	// SETUP SERVER
	// Open the socket
	if (!exit_code)
	{
		server_fd = open_socket(server_domain, server_type, server_protocol, &exit_code);
	}
	// Get an address
	if (!exit_code)
	{
		
	}
	// "Name the socket"
	if (!exit_code)
	{
		exit_code = bind_socket(server_fd, );
	}
	// Set the socket to listen
	if (!exit_code)
	{
		
	}


	// FORK
	if (!exit_code)
	{
		// Parent
		// Child
		// Error
	}

	// CLEANUP


	// DONE
	exit(exit_code);
}
