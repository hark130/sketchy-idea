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

#include <errno.h>					// EINVAL
#include <stdio.h>					// fprintf()
#include <stdlib.h>					// exit()
#include <sys/socket.h>				// AF_INET
#include <unistd.h>					// fork()
#include "skid_debug.h"				// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_descriptors.h"	// read_fd()
#include "skid_memory.h"			// free_skid_mem()
#include "skid_network.h"
#include "skid_signals.h"			// handle_all_children(), set_signal_handler()


#define SERVER_SLEEP 1  			 // Number of seconds the server sleeps while awiting connection
#define SERVER_DOMAIN AF_INET 		 // Server socket domain
#define SERVER_TYPE SOCK_STREAM		 // Server socket type
#define SERVER_PROTOCOL IPPROTO_TCP  // Server socket protocol
#define PORT "1234"					 // The port clients will connect to
#define BACKLOG 10   				 // Maximum number of pending connections to queue


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;                           // Store errno and/or results here
	int result = 0;                              // Additional errno values
	int server_domain = SERVER_DOMAIN;           // Server socket domain
	int server_type = SERVER_TYPE;               // Server socket type
	int server_protocol = SERVER_PROTOCOL;       // Server socket protocol
	int server_fd = SKID_BAD_FD;                 // Server file descriptor
	int client_fd = SKID_BAD_FD;			     // Client file descriptor
	struct addrinfo hints;                       // Selection criteria
	struct addrinfo *servinfo = NULL;            // Out argument for get_addr_info()
	struct addrinfo *temp_serv = NULL;           // Use this to walk the servinfo linked list
	struct sockaddr_storage their_addr;          // Client's address information
	socklen_t sin_size = 0;                      // The size of their_addr
	char inet_addr[INET6_ADDRSTRLEN+1] = { 0 };  // Converted socket address to human readable IP
	char *client_msg = NULL;                     // Message read from the client file descriptor
	int recv_flags = 0;                          // See recv(2)

	// INPUT VALIDATION
	if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s\n", argv[0]);
	   exit_code = EINVAL;
	}

	// SETUP SERVER
	// Setup hints
	if (!exit_code)
	{
		// Zeroize the struct
		memset(&hints, 0x0, sizeof(hints));
	    hints.ai_family = server_domain;  // AKA domain
	    hints.ai_socktype = server_type;
	    hints.ai_protocol = server_protocol;
	    hints.ai_flags = AI_PASSIVE; // use my IP
	}
	// Get an address
	if (!exit_code)
	{
		exit_code = get_addr_info(NULL, PORT, &hints, &servinfo);
	}
	// "Name the socket"
	if (!exit_code)
	{
		for (temp_serv = servinfo; NULL != temp_serv; temp_serv = temp_serv->ai_next)
		{
			// Open the socket
			server_fd = open_socket(temp_serv->ai_family, temp_serv->ai_socktype,
				                    temp_serv->ai_protocol, &exit_code);
			if (SKID_BAD_FD == server_fd)
			{
				// Failed to open the socket with this domain, type, and protocol
				PRINT_ERROR(The call to open_socket() failed);
				PRINT_ERRNO(exit_code);
				FPRINTF_ERR("%s - The call was open_socket(%d, %d, %d, %p)\n", DEBUG_ERROR_STR,
					        temp_serv->ai_family, temp_serv->ai_socktype, temp_serv->ai_protocol,
					        &exit_code);
				continue;  // Try the next node in the linked list
			}
			else
			{
				exit_code = bind_struct(server_fd, temp_serv->ai_addr, temp_serv->ai_addrlen);
				if (exit_code)
				{
					// Failed to "name the socket"
					PRINT_ERROR(The call to bind_struct() failed);
					PRINT_ERRNO(exit_code);
					FPRINTF_ERR("%s - The call was bind_struct(%d, %p, %d)\n", DEBUG_ERROR_STR,
						        server_fd, temp_serv->ai_addr, temp_serv->ai_addrlen);
					close_socket(&server_fd, false);  // This socket is no good
					continue;  // Try the next node in the linked list
				}
				else
				{
					server_protocol = temp_serv->ai_protocol;  // Store the *actual* protocol
					break;  // We got one!
				}
			}
		}
		if (NULL == temp_serv)
		{
			FPRINTF_ERR("%s - Server: failed to name a socket", DEBUG_ERROR_STR);
			exit_code = ETIMEDOUT;  // It's as close as anything else
		}
	}
	// Success or failure, we're done with the addrinfo linked list
	if (NULL != servinfo)
	{
		result = free_addr_info(&servinfo);
		if (!exit_code)
		{
			exit_code = result;
		}
	}
	// Set the socket to listen
	if (!exit_code)
	{
		if (SOCK_STREAM == server_protocol || SOCK_SEQPACKET == server_protocol)
		{
			FPRINTF_ERR("%s - Setting the server socket to listen\n", DEBUG_INFO_STR);
			exit_code = listen_socket(server_fd, BACKLOG);
		}
		else if (SOCK_DCCP == server_protocol)
		{
			FPRINTF_ERR("%s - Setting the server socket to listen for SOCK_DCCP\n",
				        DEBUG_WARNG_STR);
			exit_code = listen_socket(server_fd, BACKLOG);
		}
		else
		{
			FPRINTF_ERR("%s - Skipping 'listen' for an ineligible server socket protocol\n",
				        DEBUG_INFO_STR);  // Not an error
		}
		if (exit_code)
		{
			PRINT_ERROR(The call to listen_socket() failed);
			PRINT_ERRNO(exit_code);
		}
	}

	// ACCEPT CONNECTIONS
	if (!exit_code)
	{
		FPRINTF_ERR("%s - Server: waiting for connections...\n", DEBUG_INFO_STR);
		while(1)
		{
			// Accept
			sin_size = sizeof(their_addr);
			client_fd = accept_client(server_fd, (struct sockaddr *)&their_addr, &sin_size,
				                      &exit_code);
			if (exit_code)
			{
				FPRINTF_ERR("%s - Server: still waiting for connections...\n",
					        DEBUG_INFO_STR);
				sleep(SERVER_SLEEP);  // A tasteful sleep
				continue;  // Keep trying
			}
			else
			{
				exit_code = convert_sas_ip(&their_addr, inet_addr,
					                       INET6_ADDRSTRLEN * sizeof(char));
				if (!exit_code)
				{
					FPRINTF_ERR("%s - Server: recvd connection from %s\n",
						        DEBUG_INFO_STR, inet_addr);
					client_msg = receive_socket(client_fd, recv_flags, server_protocol, &exit_code);
					if (exit_code)
					{
						FPRINTF_ERR("%s - No message received from client connection [%d]: %s\n",
							        DEBUG_WARNG_STR, exit_code, strerror(exit_code));
					}
					else
					{
						printf("%s - Server: received message from %s: %s\n",
							   DEBUG_INFO_STR, inet_addr, client_msg);
						free_skid_mem((void **)&client_msg);  // Best effort to free temp mem
						break;  // Just accept one message
					}
				}
				else
				{
					PRINT_ERROR(The call to convert_sas_ip() failed);
					PRINT_ERRNO(exit_code);
					break;  // Stop on conversion error
				}
			}
		}
	}

	// CLEANUP
	free_skid_mem((void **)&client_msg);  // Best effort
	close_socket(&server_fd, true);  // Best effort
	close_socket(&client_fd, true);  // Best effort

	// DONE
	exit(exit_code);
}
