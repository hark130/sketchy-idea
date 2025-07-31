/*
 *  This source file was created for the express purpose of demonstrating IPC using UNIX sockets.
 *  It utilizes legacy functionality implemented in the skid_network module to:
 *      1. Create a UNIX domain socket
 *      2. Bind it to a socket file
 *      3. Listen for client connections
 *      4. Log communication to the filename specified by the args
 *
 *  Copy/paste the following...

./code/dist/test_sp_named_pipe_server.bin <NAMED_PIPE>

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <fcntl.h>                          // O_RDONLY
#include <stdbool.h>                        // true
#include <stdint.h>                         // intmax_t
#include <stdlib.h>                         // exit()
#include "skid_debug.h"                     // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_descriptors.h"          // open_fd()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // free_skid_mem()
#include "skid_pipes.h"                     // close_pipe(), delete_named_pipe(), make_named_pipe()
#include "skid_signal_handlers.h"           // handle_signal_number()
#include "skid_signals.h"                   // set_signal_handler()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define SHUTDOWN_SIG SIGINT                 // "Shutdown" signal


/*
 *  Single point of truth for this program's "escape".
 */
void print_shutdown(const char *prog_name, const char *pipe_path);

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
    int flags = O_RDONLY;        // The server reads from the pipe
    char *tmp_msg = NULL;        // Read from the pipe_fd by read_fd()
    // Mode for the named pipe
    mode_t mode = SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | SKID_MODE_GROUP_R | SKID_MODE_GROUP_W;

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
        if (ENOERR != exit_code)
        {
            goto leave_it_be;  // Don't close anything.  Don't unlink() anything.  Just leave.
        }
    }

    // SETUP
    // Signal Handler
    if (ENOERR == exit_code)
    {
        exit_code = set_signal_handler(SHUTDOWN_SIG, handle_signal_number, flags, NULL);
    }

    // DO IT
    // Make it
    if (ENOERR == exit_code)
    {
        exit_code = make_named_pipe(pipe_filename, mode);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to make_named_pipe() reported an error);
            PRINT_ERRNO(exit_code);
        }
    }
    // Open it
    if (ENOERR == exit_code)
    {
        print_shutdown(argv[0], pipe_filename);
        pipe_fd = open_fd(pipe_filename, flags, 0, &exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to open_fd() reported an error);
            PRINT_ERRNO(exit_code);
        }
    }
    // Read it
    if (ENOERR == exit_code)
    {
        print_shutdown(argv[0], pipe_filename);
        while (ENOERR == exit_code)
        {
            // SHUTDOWN_SIG?
            if (SHUTDOWN_SIG == skid_sig_hand_signum)
            {
                fprintf(stdout, "\n%s is exiting\n", argv[0]);
                break;  // Handled SHUTDOWN_SIG
            }

            // Read
            tmp_msg = read_fd(pipe_fd, &exit_code);
            if (ENOERR == exit_code)
            {
                if (strlen(tmp_msg) > 0)
                {
                    printf("SERVER: %s\n", tmp_msg);
                }
                exit_code = free_skid_mem((void **)&tmp_msg);
            }
        }
    }

    // CLEANUP
    // Named pipe file descriptor
    close_pipe(&pipe_fd, true);  // Best effort
    // Delete named pipe file
    if (NULL != pipe_filename && true == is_path(pipe_filename, &tmp_errnum))
    {
        tmp_errnum = delete_named_pipe(pipe_filename);
        if (ENOERR != tmp_errnum)
        {
            FPRINTF_ERR("%s The call to delete_named_pipe(%s) failed\n", DEBUG_ERROR_STR, pipe_filename);
            PRINT_ERRNO(tmp_errnum);
        }
    }

    // DONE
leave_it_be:
    exit(exit_code);
}


void print_shutdown(const char *prog_name, const char *pipe_path)
{
    fprintf(stdout, "%s has begun reading entries on %s\n", prog_name, pipe_path);
    fprintf(stdout, "Terminate the server by sending signal [%d] %s\n",
            SHUTDOWN_SIG, strsignal(SHUTDOWN_SIG));
    fprintf(stdout, "E.g., kill -%d %jd\n", SHUTDOWN_SIG, (intmax_t)getpid());
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <NAMED_PIPE>\n", prog_name);
}
