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
#include <sys/socket.h>		// AF_INET
#include <unistd.h>			// fork()
#include "skid_debug.h"		// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_network.h"
#include "skid_signals.h"	// handle_all_children(), set_signal_handler()


#define SERVER_DOMAIN AF_INET 		// Server socket domain
#define SERVER_TYPE SOCK_STREAM		// Server socket type
#define SERVER_PROTOCOL 0 			// Server socket protocol
#define PORT "1234"					// The port clients will connect to
#define BACKLOG 10   				// Maximum number of pending connections to queue


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
	pid_t process = 0;                           // Return value from fork()
	char message[] = { "Hello, world!" };        // Message for the client to send to the server
	ssize_t send_ret = 0;                        // Return value from send()

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
				FPRINTF_ERR("The call was open_socket(%d, %d, %d, %p)\n", temp_serv->ai_family,
					        temp_serv->ai_socktype, temp_serv->ai_protocol, &exit_code);
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
					FPRINTF_ERR("The call was bind_struct(%d, %p, %d)\n", server_fd,
						        temp_serv->ai_addr, temp_serv->ai_addrlen);
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
		else
		{
			FPRINTF_ERR("%s - Skipping 'listen' for an ineligible server socket protocol\n",
				        DEBUG_INFO_STR);  // Not an error
		}
	}

	// FORK
	// Setup signal handler
	if (!exit_code)
	{
		exit_code = set_signal_handler(SIGCHLD, handle_all_children, SA_RESTART, NULL);
	}
	// Accept
	if (!exit_code)
	{
		FPRINTF_ERR("%s - Server: waiting for connections...\n", DEBUG_INFO_STR);
		while(1)
		{
			sin_size = sizeof(their_addr);
			client_fd = accept_client(server_fd, (struct sockaddr *)&their_addr, &sin_size,
				                      &exit_code);
			if (exit_code)
			{
				FPRINTF_ERR("%s - Server: still waiting for connections...\n", DEBUG_INFO_STR);
				continue;  // I question this statement's validity but I'm leaving it in for testing
			}
			else
			{
				exit_code = convert_sas_ip(&their_addr, inet_addr, INET6_ADDRSTRLEN * sizeof(char));
				if (!exit_code)
				{
					FPRINTF_ERR("%s - Server: recvd connection from %s\n",
						        DEBUG_INFO_STR, inet_addr);
				}
			}
			// Fork
			process = fork();
			// Parent
			if (process > 0)
			{
				// Shouldn't the parent be doing something here?
			}
			// Child
			else if (0 == process)
			{
				close_socket(&server_fd, false);  // Child doesn't need the server file descriptor
				send_ret = send(client_fd, message, strlen(message), 0);
                if (send_ret < 0)
                {
					exit_code = errno;
					PRINT_ERROR(The call to send() failed);
					PRINT_ERRNO(exit_code);
                }
                else if (send_ret != strlen(message))
                {
					PRINT_ERROR(The call to send() did not send the entire message);
					FPRINTF_ERR("%s - Sent %zu bytes but expected to send %zu bytes\n",
						        DEBUG_WARNG_STR, send_ret, strlen(message));
                }
                else
                {
					FPRINTF_ERR("%s - Client: message sent!\n", DEBUG_INFO_STR);
                }
                break;  // The child has done its job
			}
			// Error
			else
			{
				exit_code = errno;
				PRINT_ERROR(The call to fork() failed);
				PRINT_ERRNO(exit_code);
				break;  // Breaking error
			}
		}
	}

	// CLEANUP
	close_socket(&server_fd, true);  // Best effort
	close_socket(&client_fd, true);  // Best effort

	// DONE
	exit(exit_code);
}
