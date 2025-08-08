/*
 *  Manually test skid_file_control's file descriptor functionality.
 *  This binary makes use of the locking functionality to lock a file indefinitely.
 *  While not ideal, this particular binary can help showcase read and write locks.
 *
 *  Copy/paste the following...

./code/dist/test_sfc_lock_and_write_fd.bin <FILENAME>
# The filename is opened and locked when the user presses <ENTER>
# Then, user input is written to the file when the input buffer is full or the user presses <ENTER>
# Use Ctrl-C to send a SIGINT signal to safely stop reading input, release the lock, close the
#   file and exit

 *
 */

#ifndef SKID_DEBUG
#define SKID_DEBUG                  // Enable DEBUG logging
#endif  /* SKID_DEBUG */

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
    int temp_retval = ENOERR;                                       // Additional errno values
    char *filename = NULL;                                          // Filename to lock and write
    int fd = SKID_BAD_FD;                                           // Filename's file descriptor
    int flags = O_CREAT | O_RDWR;                                   // Flags (see: open(2))
    size_t ui_len = 1024;                                           // Number of user_input indices
    char *user_input = NULL;                                        // User input to write to file
    char temp_input = 0x0;                                          // One keystroke
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;  // Mode (see: open(2))
    bool locked = false;                                            // Write-lock status

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
    }
    // Lock the file
    if (ENOERR == exit_code && 0 == skid_sig_hand_interrupted)
    {
        printf("The '%s' file has been opened as file descriptor number '%d'.\n", filename, fd);
        while (1)
        {
            printf("Press <ENTER> to initiate a write lock.  Press <CTRL-C> to exit.\n");
            temp_input = getchar();
            if (0 != skid_sig_hand_interrupted)
            {
                break;  // SIGINT received
            }
            else if ('\n' == temp_input)
            {
                exit_code = get_write_lock(fd);
                if (ENOERR == exit_code)
                {
                    locked = true;
                    printf("The '%d' file descriptor is now write-locked.\n", fd);
                    break;  // Got a write lock
                }
                else
                {
                    PRINT_ERROR(The call to get_write_lock() failed);
                    PRINT_ERRNO(exit_code);
                    exit_code = ENOERR;  // Let's keep trying
                }
            }
        }
        temp_input = 0x0;  // Clear the temp var
    }
    // Generate content
    if (ENOERR == exit_code && 0 == skid_sig_hand_interrupted)
    {
        printf("This program will write your text to '%s', file descriptor '%d', while it is "
               "locked for writing.\nText will be periodically written when the buffer is full "
               "or when you press <ENTER>.\nPress <CTRL-C> to exit.\nBegin typing now...\n",
               filename, fd);
    }
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
        // Write
        exit_code = write_fd(fd, user_input);
        if (ENOERR != exit_code)
        {
            FPRINTF_ERR("%s write_locked_fd(%d, %s), file descriptor for %s, failed with "
                        "errno [%d] %s\n", DEBUG_ERROR_STR, fd, user_input, filename,
                        exit_code, strerror(exit_code));
        }
        else
        {
            memset(user_input, 0x0, ui_len);  // Clear the buffer for reuse
        }
    }

    // CLEANUP
    // Release the lock
    if (true == locked)
    {
        temp_retval = release_lock(fd);
        if (ENOERR != temp_retval)
        {
            PRINT_ERROR(The call to release_lock() failed);
            PRINT_ERRNO(temp_retval);
        }
        if (ENOERR == exit_code)
        {
            exit_code = temp_retval;  // Report the first, and only the first, errno value
        }
    }
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
