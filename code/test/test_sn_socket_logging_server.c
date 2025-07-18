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
#include <stdlib.h>                         // exit()
#include "skid_debug.h"                     // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_metadata_read.h"        // is_path()
#include "skid_macros.h"                    // ENOERR

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define SOCK_PATH "/tmp/logging.sock"  // Socket filename
#define LOG_ROLLOVER 1024              // Byte threshold to backup and reset the log

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;     // Errno values
    char *log_filename = NULL;  // Log filename

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

    // SETUP
    // Signal Handler

    // LOG IT

    // DONE
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <LOG_FILENAME>\n", prog_name);
}
