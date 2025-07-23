/*
 *  This source file was created for the express purpose of demonstrating IPC using UNIX sockets.
 *  It utilizes legacy functionality implemented in the skid_network module to:
 *      1. Create a UNIX domain socket
 *      2. Bind it to a socket file
 *      3. Listen for client connections
 *      4. Log communication to the filename specified by the args
 *
 *  Copy/paste the following...

./code/dist/test_sn_named_pipe_server.bin <NAMED_PIPE>

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


/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;      // Errno values
    int tmp_errnum = ENOERR;     // Temporary errno values
    char *pipe_filename = NULL;  // Named pipe filename
    int pipe_fd = SKID_BAD_FD;   // Named pipe file descriptor

    // INPUT VALIDATION
    // Arguments
    if (2 != argc)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }
    else
    {
        pipe_filename = argv[1];
    }

    // ENVIRONMENT VALIDATION
    // Named pipe
    if (ENOERR == exit_code)
    {
        if (true == is_path(pipe_filename, &exit_code))
        {
            fprintf(stderr, "%s will not overwrite %s\n", argv[0], pipe_filename);
            exit_code = EEXIST;
        }
        else if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to is_path() reported an error);
            PRINT_ERRNO(exit_code);
        }
    }

    // LISTEN TO IT


    // CLEANUP
    // Named pipe file descriptor
    close_socket(&pipe_fd, true);  // Best effort
    // Delete SOCK_PATH
    if (NULL != pipe_filename && true == is_path(pipe_filename, &tmp_errnum))
    {
        tmp_errnum = delete_file(pipe_filename);
        if (ENOERR != tmp_errnum)
        {
            FPRINTF_ERR("%s The call to delete_file(%s) failed\n", DEBUG_ERROR_STR, pipe_filename);
            PRINT_ERRNO(tmp_errnum);
        }
    }

    // DONE
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <NAMED_PIPE>\n", prog_name);
}
