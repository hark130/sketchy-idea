/*
 *  This source file was created for the express purpose of demonstrating IPC using named pipes.
 *      1. Reads from stdin
 *      2. Opens the named pipe
 *      3. Writes to it
 *      4. Closes the named pipe
 *
 *  Copy/paste the following...

./code/dist/test_sp_named_pipe_server.bin <NAMED_PIPE>

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <fcntl.h>                          // O_WRONLY
#include <stdbool.h>                        // false
#include <stdlib.h>                         // exit()
#include <unistd.h>                         // fsync()
#include "skid_debug.h"                     // MODULE_LOAD(), MODULE_UNLOAD()
#include "skid_file_descriptors.h"          // open_fd()
#include "skid_file_metadata_read.h"        // is_named_pipe()
#include "skid_macros.h"                    // ENOERR
#include "skid_pipes.h"                     // close_pipe()

// Commented out the module load/unload to reduce verbose output in the export script
// MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
// MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#define BUF_SIZE 1024


/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;                    // Errno values
    char *pipe_filename = NULL;                // Named pipe filename
    int pipe_fd = SKID_BAD_FD;                 // Named pipe file descriptor
    int flags = O_WRONLY;                      // The client writes to the pipe
    char user_input[BUF_SIZE + 1] = { "\0" };  // Read input from the user
    char input = '\0';                         // Input character
    char *tmp_ptr = user_input;                // Iterating variable
    size_t count = 0;                          // Number of characters read

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
        if (false == is_named_pipe(pipe_filename, &exit_code))
        {
            fprintf(stderr, "%s unable to locate %s\n", argv[0], pipe_filename);
            if (ENOERR == exit_code)
            {
                exit_code = ENOENT;  // The named pipe is missing
            }
            else
            {
                PRINT_ERROR(The call to is_named_pipe() reported an error);
                PRINT_ERRNO(exit_code);
            }
        }
    }

    // DO IT
    // Start communicating
    if (ENOERR == exit_code)
    {
        while (ENOERR == exit_code)
        {
            // Only print this instruction for new statements, not wrap-arounds
            if (count < BUF_SIZE)
            {
                printf("\nType a message to the server and press <ENTER> (Blank line to exit)\n");
            }
            // Reset temp variable
            count = 0;             // Reset character count
            tmp_ptr = user_input;  // Go back to the start of the buffer
            // Take input
            while (1)
            {
                input = getchar();
                if ('\n' == input || EOF == input)
                {
                    *tmp_ptr = 0x0;  // Truncate the buffer
                    break;  // Done reading
                }
                else
                {
                    *tmp_ptr = input;  // Store the input
                    tmp_ptr++;  // Advance the pointer
                    count++;  // Increment the count
                    if (count >= BUF_SIZE)
                    {
                        *tmp_ptr = 0x0;  // Truncate the buffer
                        break;
                    }
                }
            }
            if (strlen(user_input) > 0)  // write_fd() doesn't like empty strings
            {
                // Open it
                pipe_fd = open_fd(pipe_filename, flags, 0, &exit_code);
                if (ENOERR != exit_code)
                {
                    PRINT_ERROR(The call to open_fd() reported an error);
                    PRINT_ERRNO(exit_code);
                    break;
                }
                // Write it out
                exit_code = write_fd(pipe_fd, user_input);
                if (ENOERR != exit_code)
                {
                    PRINT_ERROR(The call to write_fd() reported an error);
                    PRINT_ERRNO(exit_code);
                    break;
                }
                // Close it (to force a flush)
                exit_code = close_pipe(&pipe_fd, false);
            }
            else
            {
                break;  // I guess we're done
            }
        }
    }

    // CLEANUP
    // Named pipe file descriptor
    close_pipe(&pipe_fd, true);  // Best effort

    // DONE
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <NAMED_PIPE>\n", prog_name);
}
