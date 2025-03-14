/*
 *  Manually test skid_file_control's file descriptor functionality.
 *  This binary makes use of the write_locked_fd() to lock, write to file, and release.
 *
 *  Copy/paste the following...

./code/dist/test_sfc_write_locked_fd.bin <FILENAME>
# User input is written to the locked filename when the buffer is full or the user presses <ENTER>
# Use Ctrl-C to send a SIGINT signal to safely exit

 *
 */

#ifndef SKID_DEBUG
#define SKID_DEBUG                  // Enable DEBUG logging
#endif  /* SKID_DEBUG */
#define WAIT_SLEEP 10               // Number of seconds to wait before attempting another lock

#include <errno.h>                  // EINVAL
#include <fcntl.h>                  // O_CREAT, O_WRONLY
#include <stdio.h>                  // printf()
#include <stdlib.h>                 // exit()
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
    int exit_code = ENOERR;                                         // Errno values
    char *filename = NULL;                                          // Filename to lock and write
    int fd = SKID_BAD_FD;                                           // Filename's file descriptor
    int flags = O_CREAT | O_RDWR;                                   // Flags (see: open(2))
    size_t ui_len = 1024;                                           // Number of user_input indices
    char *user_input = NULL;                                        // User input to write to file
    char temp_input = 0x0;                                          // One keystroke
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;  // Mode (see: open(2))

    // INPUT VALIDATION
    if (argc == 2)
    {
        filename = argv[1];  // Filename to write to
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
    // Allocate a read buffer
    if (ENOERR == exit_code)
    {
        user_input = alloc_skid_mem(ui_len + 1, sizeof(char), &exit_code);
    }

    // WRITE IT
    // Open the filename
    if (ENOERR == exit_code)
    {
        fd = open_fd(filename, flags, mode, &exit_code);
        if (SKID_BAD_FD == fd || ENOERR != exit_code)
        {
            FPRINTF_ERR("%s open_fd(%s, %d, %d, %p) failed with errno [%d] %s\n", DEBUG_ERROR_STR,
                        filename, flags, mode, &exit_code, exit_code, strerror(exit_code));
        }
        else
        {
            printf("This program will write your text to '%s' while it is locked for writing.\n"
                   "Text will be periodically written when the buffer is full "
                   "or when you press <ENTER>.\nPress <CTRL-C> to exit.\nBegin typing now...\n",
                   filename);
        }
    }
    // Generate content
    while (ENOERR == exit_code && 0 == skid_sig_hand_interrupted)
    {
        // Take user input until the buffer is full or the user ends it
        for (int i = 0; i < ui_len; i++)
        {
            temp_input = getchar();
            user_input[i] = temp_input;
            if (EOF == temp_input || '\n' == temp_input || 0 != skid_sig_hand_interrupted)
            {
                break;
            }
        }
        // Lock and write
        while(0 == skid_sig_hand_interrupted)
        {
            exit_code = write_locked_fd(fd, user_input);
            if (ENOERR == exit_code)
            {
                memset(user_input, 0x0, ui_len);  // Clear the buffer for reuse
                break;  // Done
            }
            else if (EAGAIN == exit_code)
            {
                FPRINTF_ERR("%s Operation is prohibited by locks held by other processes: "
                            "[%d] %s\n", DEBUG_INFO_STR, exit_code, strerror(exit_code));
            }
            else if (EACCES == exit_code)
            {
                FPRINTF_ERR("%s Operation may be prohibited by locks held by other processes: "
                            "[%d] %s\n", DEBUG_WARNG_STR, exit_code, strerror(exit_code));
            }
            else
            {
                FPRINTF_ERR("%s write_locked_fd(%d, %s), file descriptor for %s, failed with "
                            "errno [%d] %s\n", DEBUG_ERROR_STR, fd, user_input, filename,
                            exit_code, strerror(exit_code));
                break;  // Unexpected error
            }
            // Reset and wait
            if (ENOERR != exit_code)
            {
                printf("Write-lock failed.  Will try again.  Press <CTRL-C> to exit.\n");
                exit_code = ENOERR;  // Reset
                sleep(WAIT_SLEEP);  // A tasteful sleep
                printf("Continue typing...\n");
            }
        }
    }

    // CLEANUP
    // file_content
    if (NULL != user_input)
    {
        free_skid_mem((void **)&user_input);  // Best effort
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
