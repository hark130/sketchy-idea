/*
 *  This source file was created to demonstrate the proper usage of poll.
 *      1. Sets up a "shutdown" signal handler
 *      2. Spawns a swarm of children, each with a dedicated unnamed pipe
 *      3. Maintains bookkeeping: PIDs, pipe fds, etc.
 *      4. Each child will write to their unnamed pipe at psuedo-random intervals
 *      5. The parent will use poll to watch the read ends of the pipes
 *      6. It all gets shutdown when the signal is received
 *
 *  Copy/paste the following...

./code/dist/test_misc_child_swarm_poll.bin <NUM_CHILDREN>

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <inttypes.h>                       // strtoumax()
#include <stdbool.h>                        // false
#include <stdio.h>                          // fprintf()
#include <stdlib.h>                         // exit()
#include <unistd.h>                         // fork()
#include "skid_debug.h"                     // MODULE_*LOAD(), *PRINT*_ERR*()
#include "skid_macros.h"                    // ENOERR, SKID_BAD_FD, SKID_BAD_PID
#include "skid_signal_handlers.h"           // handle_signal_number()
#include "skid_signals.h"                   // set_signal_handler()
#include "skid_validation.h"                // validate_skid_*()

#define CHILD_STR "CHILD"                   // Identifying string for child process logging
#define PARENT_STR "PARENT"                 // Identifying string for parent process logging
#define MAIN_STR "MAIN"                     // Identifying string for .bin logging
#define SHUTDOWN_SIG SIGINT                 // "Shutdown" signal

/*
 *  Start randomizing intervals and writing to fd in an infinite loop... like a child.
 */
void be_a_child(int fd);

/*
 *  Description:
 *      Convert a string to an unsigned integer.
 *
 *  Args:
 *      string: A non-empty string to convert.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Converted value on success.  On error, 0 and errnum is set.
 */
unsigned int convert_str_to_uint(const char *string, int *errnum);

/*
 *  Print shutdown instructions.
 */
void print_shutdown(const char *prog_name);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;         // Errno values
    unsigned int num_children = 0;  // Number of children to spawn
    pid_t pid = SKID_BAD_PID;       // Temp return value from fork()

    // INPUT VALIDATION
    if (2 != argc)
    {
       exit_code = EINVAL;
    }
    else
    {
        num_children = convert_str_to_uint(argv[1], &exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to convert_str_to_uint() reported an error);
            FPRINTF_ERR("%s Unable to convert '%s' to an unsigned int\n", DEBUG_ERROR_STR, argv[1]);
            PRINT_ERRNO(exit_code);
        }
        else if (num_children <= 0)
        {
            PRINT_ERROR(The <NUM_CHILDREN> value must be positive);
            exit_code = EINVAL;
        }
    }
    if (ENOERR != exit_code)
    {
       print_usage(argv[0]);
    }

    // SETUP
    // Signal Handler
    if (ENOERR == exit_code)
    {
        exit_code = set_signal_handler(SHUTDOWN_SIG, handle_signal_number, 0, NULL);
    }

    // DO IT
    // Fork it
    if (ENOERR == exit_code)
    {
        for (unsigned int i = 0; i < num_children; i++)
        {
            // Create a temp, local pipe
            // Fork
            pid = fork();
            // Parent
            if (pid > 0)
            {
                // Store the PID
                // Close the write end of the pipe
                // Add the read end of the pipe to the bookkeeping
            }
            // Child
            else if (0 == pid)
            {
                // Close the read end of the pipe
                // Start logging
                be_a_child(SKID_BAD_FD);
            }
            // Error
            else
            {
                exit_code = errno;
                PRINT_ERROR(The call to fork() reported an error);
                PRINT_ERRNO(exit_code);
                break;  // We encountered an error, so stop looping
            }

        }
    }

    // Start poll()ing (in the parent)
    if (ENOERR == exit_code && pid > 0)
    {
        print_shutdown(argv[0]);
        fprintf(stdout, "\n\n%s - Polling...", PARENT_STR);
        while(1)
        {
            if (SHUTDOWN_SIG == skid_sig_hand_signum)
            {
                fprintf(stdout, "\n%s is exiting\n", argv[0]);
                break;  // Time to cleanup and exit
            }
            // poll() here
            sleep(1);
            fprintf(stdout, ".");
            fflush(stdout);
        }
    }

    // CLEANUP

    // DONE
    exit(exit_code);
}


void be_a_child(int fd)
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;  // Errno values

    // INPUT VALIDATION
    exit_code = validate_skid_fd(fd);

    // BE IT
    while (ENOERR == exit_code)
    {
        if (SHUTDOWN_SIG == skid_sig_hand_signum)
        {
            break;  // Time to cleanup and exit
        }
        // Randomize a number
        // Sleep
        // Generate a messge
        // Write the message to the fd
    }

    // CLEANUP
    // close_pipe_fd(&fd, false);  // Close the pipe file descriptor

    // DONE
    exit(exit_code);
}


unsigned int convert_str_to_uint(const char *string, int *errnum)
{
    // LOCAL VARIABLES
    unsigned int converted_val = 0;  // Converted value
    int results = 0;                 // Store errno value

    // INPUT VALIDATION
    results = validate_skid_string(string, false);  // Can't be empty
    if (ENOERR == results)
    {
        results = validate_skid_err(errnum);
    }

    // CONVERT IT
    if (ENOERR == results)
    {
        converted_val = strtoumax(string, NULL, 10);
        if (UINTMAX_MAX == converted_val)
        {
            results = errno;  // ERANGE or valid?
        }
    }

    // DONE
    if (ENOERR != results)
    {
        converted_val = 0;
    }
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return converted_val;
}


void print_shutdown(const char *prog_name)
{
    fprintf(stdout, "%s has finished spawning children and is now poll()ing pipes\n",
            prog_name);
    fprintf(stdout, "Terminate the server by sending signal [%d] %s\n",
            SHUTDOWN_SIG, strsignal(SHUTDOWN_SIG));
    fprintf(stdout, "E.g., kill -%d %jd\n", SHUTDOWN_SIG, (intmax_t)getpid());
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <NUM_CHILDREN>\n", prog_name);
}
