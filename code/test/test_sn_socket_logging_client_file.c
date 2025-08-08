/*
 *  This source file was created for the express purpose of demonstrating IPC using UNIX sockets.
 *  It utilizes legacy functionality implemented in the skid_network module to:
 *      1. Create a UNIX domain socket
 *      2. Bind it to a socket file
 *      3. Listen for client connections
 *      4. Log communication to the filename specified by the args
 *
 *  Copy/paste the following...

./code/dist/test_sn_socket_logging_client_file.bin <LOG_FILENAME>

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <stdbool.h>                        // true
#include <stdlib.h>                         // exit()
#include <sys/socket.h>                     // AF_UNIX
#include <sys/un.h>                         // struct sockaddr_un
#include "skid_debug.h"                     // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_descriptors.h"          // write_fd()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_file_operations.h"           // read_file()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // free_skid_mem()
#include "skid_network.h"                   // close_socket()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define SOCK_PATH "/tmp/logging.sock"  // Socket filename
#define SOCKET_DOMAIN AF_UNIX          // Socket domain
#define SOCKET_TYPE SOCK_STREAM        // Socket type

/*
 *  Open a socket, binds it to sock_path and sets it to listen.  Returns the socket file descriptor.
 */
int connect_socket_file(int domain, int type, int protocol, const char *sock_path, int *errnum);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;     // Errno values
    char *in_file = NULL;       // Filename to use as input
    char *in_cont = NULL;       // Input file contents
    int sock_fd = SKID_BAD_FD;  // Socket file descriptor

    // INPUT VALIDATION
    // Arguments
    if (2 != argc)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }
    else
    {
        in_file = argv[1];
    }

    // ENVIRONMENT VALIDATION
    // Log filename
    if (ENOERR == exit_code)
    {
        if (false == is_path(in_file, &exit_code))
        {
            fprintf(stderr, "%s unable to locate '%s'\n", argv[0], in_file);
            if (ENOERR == exit_code)
            {
                exit_code = ENOENT;
            }
            else
            {
                PRINT_ERROR(The call to is_path() failed);
                PRINT_ERRNO(exit_code);
            }
        }
    }
    // Socket filename
    if (ENOERR == exit_code)
    {
        if (false == is_socket(SOCK_PATH, &exit_code))
        {
            fprintf(stderr, "%s Unable to locate socket file '%s'\n", DEBUG_WARNG_STR, SOCK_PATH);
            exit_code = ENOENT;
        }
    }

    // SETUP
    // Setup the socket
    if (ENOERR == exit_code)
    {
        sock_fd = connect_socket_file(SOCKET_DOMAIN, SOCKET_TYPE, 0, SOCK_PATH, &exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to setup_socket() failed);
            PRINT_ERRNO(exit_code);
        }
    }

    // WRITE
    // Read from input file
    if (ENOERR == exit_code)
    {
        in_cont = read_file(in_file, &exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to read_file() failed);
            PRINT_ERRNO(exit_code);
        }
    }
    if (ENOERR == exit_code)
    {
        exit_code = write_fd(sock_fd, in_cont);
    }

    // CLEANUP
    if (NULL != in_cont)
    {
        free_skid_mem((void **)&in_cont);  // Best effort
    }
    // Socket file descriptor
    close_socket(&sock_fd, true);  // Best effort

    // DONE
    exit(exit_code);
}


int connect_socket_file(int domain, int type, int protocol, const char *sock_path, int *errnum)
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

    // Connect to it
    if (ENOERR == results)
    {
        addr.sun_family = domain;
        strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);
        results = connect_socket(sock_fd, (struct sockaddr *)&addr, addr_size);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to connect_socket() failed);
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


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <INPUT_FILENAME>\n", prog_name);
}
