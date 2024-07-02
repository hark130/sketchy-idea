/*
 *	Manually test skid_network's manual socket, bind, and listen functions.
 *	This binary use the function wrappers to replicate this behavior:
 *		https://github.com/beejjorgensen/bgnet/blob/main/src/bgnet_part_0600_clientserver.md
 *
 *	Copy/paste the following...

# All values are hard-coded so no arguments are necessary
./code/dist/test_sn_simple_stream_client.bin

 *
 */

#define SKID_DEBUG					// Enable DEBUG logging

#include <errno.h>					// EINVAL
#include <stdio.h>					// fprintf()
#include <stdlib.h>					// exit()
#include <sys/socket.h>				// AF_INET
#include <unistd.h>					// fork()
#include "skid_debug.h"				// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_descriptors.h"	// write_fd()
#include "skid_network.h"
#include "skid_signals.h"			// handle_all_children(), set_signal_handler()


#define SERVER_SLEEP 1  			// Number of seconds the server sleeps while awiting connection
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
	int server_fd = SKID_BAD_FD;                 // Server file descriptor
	struct addrinfo hints;                       // Selection criteria
	struct addrinfo *servinfo = NULL;            // Out argument for get_addr_info()
	struct addrinfo *temp_serv = NULL;           // Use this to walk the servinfo linked list
	char message[] = { "Hello, world!" };        // Message for the client to send to the server
	// ssize_t send_ret = 0;                        // Return value from send()

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
	// Connect
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
				exit_code = connect_socket(server_fd, temp_serv->ai_addr, temp_serv->ai_addrlen);
				if (exit_code)
				{
					// Failed to connect the socket to the address
					PRINT_ERROR(The call to connect_socket() failed);
					PRINT_ERRNO(exit_code);
					FPRINTF_ERR("The call was connect_socket(%d, %p, %d)\n", server_fd,
						        temp_serv->ai_addr, temp_serv->ai_addrlen);
					close_socket(&server_fd, false);  // This socket is no good
					continue;  // Try the next node in the linked list
				}
				else
				{
					break;  // We got one!
				}
			}
		}
		if (NULL == temp_serv)
		{
			FPRINTF_ERR("%s - Client: failed to connect to the server\n", DEBUG_ERROR_STR);
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

	// CLIENT
	if (!exit_code)
	{
		FPRINTF_ERR("%s - Client: attempting to send data...\n", DEBUG_INFO_STR);
		// send_ret = send(server_fd, message, strlen(message), 0);
        // if (send_ret < 0)
        // {
		// 	exit_code = errno;
		// 	PRINT_ERROR(The call to send() failed);
		// 	PRINT_ERRNO(exit_code);
        // }
        // else if (send_ret != strlen(message))
        // {
		// 	PRINT_ERROR(The call to send() did not send the entire message);
		// 	FPRINTF_ERR("%s - Sent %zu bytes but expected to send %zu bytes\n",
		// 		        DEBUG_WARNG_STR, send_ret, strlen(message));
        // }
        // else
        // {
		// 	FPRINTF_ERR("%s - Client: message sent!\n", DEBUG_INFO_STR);
        // }
        exit_code = write_fd(server_fd, message);
        if (!exit_code)
        {
			FPRINTF_ERR("%s - Client: message sent!\n", DEBUG_INFO_STR);
        }
        else
        {
			PRINT_ERROR(The call to write_fd() failed);
			PRINT_ERRNO(exit_code);
        }
        // exit_code = send_socket(server_fd, message, 0);
        // if (!exit_code)
        // {
		// 	FPRINTF_ERR("%s - Client: message sent!\n", DEBUG_INFO_STR);
        // }
        // else
        // {
		// 	PRINT_ERROR(The call to send_socket() failed);
		// 	PRINT_ERRNO(exit_code);
        // }
	}

	// CLEANUP
	close_socket(&server_fd, true);  // Best effort

	// DONE
	exit(exit_code);
}
