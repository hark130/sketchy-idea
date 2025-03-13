/*
 *  Manually test skid_file_control's file descriptor functionality.
 *  This binary makes use of the read_locked_fd() to repeatedly read a file.
 *
 *  Copy/paste the following...

./code/dist/test_sfc_read_locked_fd.bin <FILENAME>
# Use Ctrl-C to send a SIGINT signal to safely exit

 *
 */

#ifndef SKID_DEBUG
#define SKID_DEBUG                  // Enable DEBUG logging
#endif  /* SKID_DEBUG */
#define WAIT_SLEEP 10               // Number of seconds to wait for user input

#include <errno.h>                   // EINVAL
#include <stdio.h>                   // printf()
#include <stdlib.h>                  // exit()
#include <unistd.h>                 // sleep()
#include "skid_debug.h"             // FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()
#include "skid_file_control.h"      // read_locked_fd()
#include "skid_file_descriptors.h"  // close_fd(), open_fd()
#include "skid_macros.h"            // ENOERR, SKID_BAD_FD
#include "skid_memory.h"            // free_skid_mem()
#include "skid_signals.h"           // set_signal_handler()
#include "skid_signal_handlers.h"   // handle_interruptions()

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;     // Store errno and/or results here
    int result = ENOERR;        // Additional errno values
    char *filename = NULL;      // The filename to lock, read, and release
    int fd = SKID_BAD_FD;       // Filename's file descriptor
    int flags = 0;              // Flags (see: open(2))
    mode_t mode = 0;            // Mode (see: open(2))
    char *file_content = NULL;  // Contents of filename

    // INPUT VALIDATION
    if (argc == 2)
    {
        filename = argv[1];  // Filename to read
    }
    else
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }

    // SETUP
    // Setup a SIGINT signal handler to gracefully handle interruptions
    if (ENOERR == exit_code)
    {
        exit_code = set_signal_handler(SIGINT, handle_interruptions, 0, NULL);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to set_signal_handler() failed);
            PRINT_ERRNO(exit_code);
        }
    }

    // READ IT
    while (ENOERR == exit_code && 0 == skid_sig_hand_interrupted)
    {
        // Open the filename
        fd = open_fd(filename, flags, mode, &exit_code);
        if (SKID_BAD_FD == fd)
        {
            FPRINTF_ERR("%s open_fd(%s, %d, %d, %p) failed with errno [%d] %s\n", DEBUG_ERROR_STR,
                        filename, flags, mode, &exit_code, exit_code, strerror(exit_code));
        }
        // Lock and read it
        else
        {
            file_content = read_locked_fd(fd, &exit_code);
            if (NULL == file_content || ENOERR != exit_code)
            {
                FPRINTF_ERR("%s read_locked_fd(%d, %p), file descriptor for %s, failed with "
                            "errno [%d] %s\n", DEBUG_ERROR_STR, fd, &exit_code, filename,
                            exit_code, strerror(exit_code));
            }
            else
            {
                printf("\nCONTENTS:\n%s\n", file_content);
                result = free_skid_mem((void **)&file_content);
                if (ENOERR == exit_code)
                {
                    exit_code = result;  // Report the first, and only the first, error
                }
            }
            // Close the file descriptor
            if (SKID_BAD_FD != fd)
            {
                result = close_fd(&fd, false);
                if (ENOERR == exit_code)
                {
                    exit_code = result;  // Report the first, and only the first, error
                }
            }
        }
        // Sleep?
        if (ENOERR == exit_code)
        {
            printf("Sleeping %d seconds to read again.  Press <CTRL-C> to exit.\n", WAIT_SLEEP);
            for (int i = 0; i < WAIT_SLEEP; i++)
            {
                if (0 != skid_sig_hand_interrupted)
                {
                    break;  // Received a SIGINT so let's bail
                }
                else
                {
                    sleep(i);  // A tasteful sleep
                }
            }
        }
    }

    // CLEANUP
    // file_content
    if (NULL != file_content)
    {
        free_skid_mem((void **)&file_content);  // Best effort
    }
    // fd
    close_fd(&fd, true);  // Best effort

    // DONE
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <FILENAME>\n", prog_name);
}
