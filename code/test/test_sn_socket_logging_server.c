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
#include "skid_network.h"                   // close_socket()
#include "skid_signals.h"                   // set_signal_handler()
#include "skid_signal_handlers.h"           // handle_signal_number()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define LOG_ROLLOVER 1024              // Byte threshold to backup and reset the log
#define SOCK_PATH "/tmp/logging.sock"  // Socket filename
#define SHUTDOWN_SIG SIGINT            // "Shutdown" signal
#define SOCKET_DOMAIN AF_UNIX          // Socket domain
#define SOCKET_TYPE SOCK_STREAM        // Socket type

/*
 *  Single point of truth for this program's "escape".
 */
void print_shutdown(const char *prog_name, const char *sock_path);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Open the socket and bind it to sock_path.  Returns the socket file descriptor.
 */
int setup_socket(int domain, int type, int protocol, const char *sock_path, int *errnum);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;           // Errno values
    char *log_filename = NULL;        // Log filename
    int flags = 0;                    // Modifies signal behavior: none necessary(?)
    int sock_fd = SKID_BAD_FD;        // Socket file descriptor
    int tmp_errnum = ENOERR;          // Temp errnum values

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

    // List on the socket
    if (ENOERR == exit_code)
    {
        print_shutdown(argv[0], SOCK_PATH);
        while (1)
        {


            // SHUTDOWN_SIG?
            if (SHUTDOWN_SIG == skid_sig_hand_signum)
            {
                fprintf(stdout, "\n%s is exiting\n", argv[0]);  // DEBUGGING?
                break;  // Handled SHUTDOWN_SIG
            }

            sleep(1);  // Tasteful sleep
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
