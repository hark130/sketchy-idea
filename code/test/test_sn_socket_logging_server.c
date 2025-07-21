/*
 *  This source file was created for the express purpose of demonstrating IPC using UNIX sockets.
 *  It utilizes legacy functionality implemented in the skid_network module to:
 *      1. Create a UNIX domain socket
 *      2. Bind it to a socket file
 *      3. Listen for client connections
 *      4. Log communication to the filename specified by the args
 *
 *  Copy/paste the following...

./code/dist/test_sn_socket_logging_server.bin <LOG_FILENAME>

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <stdbool.h>                        // true
#include <stdint.h>                         // intmax_t
#include <stdlib.h>                         // exit()
#include <sys/socket.h>                     // AF_UNIX
#include <sys/un.h>                         // struct sockaddr_un
#include "skid_debug.h"                     // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_file_operations.h"           // delete_file()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // free_skid_mem()
#include "skid_network.h"                   // close_socket()
#include "skid_signals.h"                   // set_signal_handler()
#include "skid_signal_handlers.h"           // handle_signal_number()
#include "skid_time.h"                      // timestamp_a_msg()
#include "skid_validation.h"                // validate_skid_pathname()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define LOG_ROLLOVER 1024              // Byte threshold to backup and reset the log
#define SOCK_PATH "/tmp/logging.sock"  // Socket filename
#define SHUTDOWN_SIG SIGINT            // "Shutdown" signal
#define SOCKET_DOMAIN AF_UNIX          // Socket domain
#define SOCKET_TYPE SOCK_STREAM        // Socket type
#define QUEUE_BACKLOG (int)1024        // Maximum length of the pending socket queue

/*
 *  Log a message.
 */
int log_message(const char *log_filename, const char *msg);

/*
 *  Single point of truth for this program's "escape".
 */
void print_shutdown(const char *prog_name, const char *sock_path);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Read a message from a socket file descriptor, log it to log_filename, and close sockfd.
 */
int receive_and_log(int *sockfd, int flags, const char *log_filename);

/*
 *  Open a socket, binds it to sock_path and sets it to listen.  Returns the socket file descriptor.
 */
int setup_socket(int domain, int type, int protocol, const char *sock_path, int *errnum);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;           // Errno values
    char *log_filename = NULL;        // Log filename
    int flags = 0;                    // Modifies signal behavior: none necessary(?)
    int sock_fd = SKID_BAD_FD;        // Socket file descriptor
    int client_fd = SKID_BAD_FD;      // Incoming client file descriptor
    int tmp_errnum = ENOERR;          // Temp errnum values
    int recv_flags = 0;               // See recv(2)

    // INPUT VALIDATION
    // Arguments
    if (2 != argc)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }
    else
    {
        log_filename = argv[1];
    }

    // ENVIRONMENT VALIDATION
    // Log filename
    if (ENOERR == exit_code)
    {
        if (true == is_path(log_filename, &exit_code))
        {
            fprintf(stderr, "%s will not overwrite %s\n", argv[0], log_filename);
            exit_code = EEXIST;
        }
        else if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to is_path() reported an error);
            PRINT_ERRNO(exit_code);
        }
    }
    // Socket filename
    if (ENOERR == exit_code)
    {
        if (true == is_path(SOCK_PATH, &exit_code))
        {
            fprintf(stderr, "%s %s exists\n", DEBUG_WARNG_STR, log_filename);
        }
        else if (ENOERR != exit_code)
        {
            FPRINTF_ERR("%s The call to is_path() reported a problem with %s\n",
                        DEBUG_ERROR_STR, SOCK_PATH);
            PRINT_ERRNO(exit_code);
        }
    }

    // SETUP
    // Signal Handler
    if (ENOERR == exit_code)
    {
        exit_code = set_signal_handler(SHUTDOWN_SIG, handle_signal_number, flags, NULL);
    }

    // LOG IT
    // Setup the socket
    if (ENOERR == exit_code)
    {
        sock_fd = setup_socket(SOCKET_DOMAIN, SOCKET_TYPE, 0, SOCK_PATH, &exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to setup_socket() failed);
            PRINT_ERRNO(exit_code);
        }
    }

    // Listen on the socket
    if (ENOERR == exit_code)
    {
        print_shutdown(argv[0], SOCK_PATH);
        while (1)
        {
            // SHUTDOWN_SIG?
            if (SHUTDOWN_SIG == skid_sig_hand_signum)
            {
                fprintf(stdout, "\n%s is exiting\n", argv[0]);
                break;  // Handled SHUTDOWN_SIG
            }

            client_fd = accept_client(sock_fd, NULL, 0, &exit_code);
            if (EINVAL == exit_code)
            {
                PRINT_ERROR(The call to accept_client() failed validation);
                PRINT_ERRNO(exit_code);
                break;
            }
            else if (ENOERR != exit_code)
            {
                exit_code = ENOERR;  // Still waiting on an incoming connection
                sleep(1);  // Tasteful sleep
            }
            else
            {
                exit_code = receive_and_log(&client_fd, recv_flags, log_filename);
            }
        }
    }

    // CLEANUP
    // Socket file descriptor
    close_socket(&sock_fd, true);  // Best effort
    // Delete SOCK_PATH
    if (true == is_path(SOCK_PATH, &tmp_errnum))
    {
        tmp_errnum = delete_file(SOCK_PATH);
        if (ENOERR != tmp_errnum)
        {
            FPRINTF_ERR("%s The call to delete_file(%s) failed\n", DEBUG_ERROR_STR, SOCK_PATH);
            PRINT_ERRNO(tmp_errnum);
        }
    }

    // DONE
    exit(exit_code);
}


int log_message(const char *log_filename, const char *msg)
{
    // LOCAL VARIABLES
    int results = ENOERR;           // Errno values
    char *stamp_msg = NULL;         // Time-stamped message
    char delims[2] = { '[', ']' };  // Timestamp delimiters

    // INPUT VALIDATION
    // Log filename
    if (ENOERR == results)
    {
        results = validate_skid_pathname(log_filename, false);
    }
    // msg
    if (ENOERR == results)
    {
        results = validate_skid_string(msg, true);
    }

    // LOG IT
    // Form the time-stamped message
    if (ENOERR == results)
    {
        stamp_msg = timestamp_a_msg(msg, delims, &results);
    }
    // Write the time-stamped message to log_filename
    if (ENOERR == results)
    {
        results = append_to_file(log_filename, stamp_msg, true);
    }

    // CLEAN UP
    if (NULL != stamp_msg)
    {
        free_skid_mem((void**)&stamp_msg);  // Best effort
    }

    // DONE
    return results;
}


void print_shutdown(const char *prog_name, const char *sock_path)
{
    fprintf(stdout, "%s has begun listening for log entries on %s\n", prog_name, sock_path);
    fprintf(stdout, "Terminate the server by sending signal [%d] %s\n",
            SHUTDOWN_SIG, strsignal(SHUTDOWN_SIG));
    fprintf(stdout, "E.g., kill -%d %jd\n", SHUTDOWN_SIG, (intmax_t)getpid());
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <LOG_FILENAME>\n", prog_name);
}


int receive_and_log(int *sockfd_ptr, int flags, const char *log_filename)
{
    // LOCAL VARIABLES
    int results = ENOERR;      // Errno values
    int sockfd = SKID_BAD_FD;  // Local copy of the socket file descriptor
    char *msg = NULL;          // Message read from the socket file descriptor

    // INPUT VALIDATION
    // Socket file descriptor
    if (NULL == sockfd_ptr)
    {
        results = EINVAL;
    }
    else
    {
        sockfd = *sockfd_ptr;
        results = validate_skid_sockfd(sockfd);
    }
    // Log filename
    if (ENOERR == results)
    {
        results = validate_skid_pathname(log_filename, false);
    }

    // DO IT
    // Receive
    if (ENOERR == results)
    {

        msg = receive_socket(sockfd, flags, SOCK_STREAM, &results);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to receive_socket() failed);
            PRINT_ERRNO(results);
        }
    }
    // Log
    if (ENOERR == results)
    {
        results = log_message(log_filename, msg);
    }

    // CLEAN UP
    // Close the socket
    if (NULL != sockfd_ptr && SKID_BAD_FD != *sockfd_ptr)
    {
        if (ENOERR == results)
        {
            results = close_socket(sockfd_ptr, false);
        }
        else
        {
            close_socket(sockfd_ptr, false);  // Don't overwrite the original error
        }
    }
    // Free the message
    if (NULL != msg)
    {
        free_skid_mem((void**)&msg);  // Best effort
    }

    // DONE
    return results;
}


int setup_socket(int domain, int type, int protocol, const char *sock_path, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;             // Errno values
    int sock_fd = SKID_BAD_FD;        // Socket file descriptor
    struct sockaddr_un addr;          // bind_struct() argument
    size_t addr_size = sizeof(struct sockaddr_un);  // Size of sockaddr_un

    // INPUT VALIDATION
    if (strlen(sock_path) * sizeof(sock_path[0]) > sizeof(addr.sun_path) - 1)
    {
        PRINT_ERROR(The socket path is too long to copy into the sockaddr struct);
        results = ENAMETOOLONG;
    }

    // SETUP
    // Open it
    if (ENOERR == results)
    {
        memset(&addr, 0x0, addr_size);  // Zeroize the struct

        // Open the socket
        sock_fd = open_socket(SOCKET_DOMAIN, SOCKET_TYPE, 0, &results);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to open_socket() failed);
            PRINT_ERRNO(results);
        }
    }

    // Bind it
    if (ENOERR == results)
    {
        addr.sun_family = domain;
        strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);
        results = bind_struct(sock_fd, (struct sockaddr *)&addr, addr_size);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to bind_struct() failed);
            PRINT_ERRNO(results);
        }
    }

    // Set it to listen
    if (ENOERR == results)
    {
        results = listen_socket(sock_fd, QUEUE_BACKLOG);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to listen_socket() failed);
            PRINT_ERRNO(results);
        }
    }

    // CLEANUP
    if (ENOERR != results)
    {
        close_socket(&sock_fd, true);  // Best effort
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return sock_fd;
}
