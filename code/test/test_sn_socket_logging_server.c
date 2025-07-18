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
#include "skid_debug.h"                     // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_macros.h"                    // ENOERR
#include "skid_signals.h"                   // set_signal_handler()
#include "skid_signal_handlers.h"           // handle_signal_number()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define LOG_ROLLOVER 1024              // Byte threshold to backup and reset the log
#define SOCK_PATH "/tmp/logging.sock"  // Socket filename
#define SHUTDOWN_SIG SIGINT            // "Shutdown" signal

/*
 *  Single point of truth for this program's "escape".
 */
void print_shutdown(const char *prog_name, const char *sock_path);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;           // Errno values
    char *log_filename = NULL;        // Log filename
    int flags = 0;                    // Modifies signal behavior: none necessary(?)

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
    // Delete SOCK_PATH

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
