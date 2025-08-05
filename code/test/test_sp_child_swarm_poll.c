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
#include <fcntl.h>                          // O_NONBLOCK
#include <inttypes.h>                       // strtoumax()
#include <stdbool.h>                        // false
#include <stdio.h>                          // fprintf()
#include <stdlib.h>                         // exit()
#include <unistd.h>                         // fork()
#include "skid_debug.h"                     // MODULE_*LOAD(), *PRINT*_ERR*()
#include "skid_file_descriptors.h"          // read_fd(), write_fd()
#include "skid_macros.h"                    // ENOERR, SKID_BAD_FD, SKID_BAD_PID
#include "skid_memory.h"                    // free_skid_mem()
#include "skid_pipes.h"                     // close_pipe(), create_pipes()
#include "skid_poll.h"                      // struct pollfd
#include "skid_random.h"                    // randomize_number()
#include "skid_signal_handlers.h"           // handle_signal_number()
#include "skid_signals.h"                   // set_signal_handler()
#include "skid_time.h"                      // timestamp_a_msg()
#include "skid_validation.h"                // validate_skid_*()

#define CHILD_STR "CHILD"                   // Identifying string for child process logging
#define PARENT_STR "PARENT"                 // Identifying string for parent process logging
#define MAIN_STR "MAIN"                     // Identifying string for .bin logging
#define SHUTDOWN_SIG SIGINT                 // "Shutdown" signal
#define MAX_SLEEP 10                        // Upper end bound for random sleep() values

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
 *  "[20250721-124356] Hello from PID <PID>"
 *  Use free_skid_mem() on the return value.
 */
char *generate_a_message(int *errnum);

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
    int exit_code = ENOERR;           // Errno values
    unsigned int num_children = 0;    // Number of children to spawn
    pid_t pid = SKID_BAD_PID;         // Temp return value from fork()
    int pipe_read_fd = SKID_BAD_FD;   // Temp read end of the pipe
    int pipe_write_fd = SKID_BAD_FD;  // Temp write end of the pipe
    // int *fds = NULL;                  // Heap-allocated array for the parent to store read_fds
    pid_t *child_pids = NULL;         // Heap-allocated array for the parent to store child PIDs
    struct pollfd *poll_fds = NULL;   // Heap-allocated array for the pollfd structs
    char *tmp_msg = NULL;             // Temp var w/ heap-allocated string read for poll()ing fds
    int num_rdy = 0;                  // Number of fds ready
    int tmp_revents = 0;              // Temp var to store the revents

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
    // // Parent's array of pipe read file descriptors
    // if (ENOERR == exit_code)
    // {
    //     fds = alloc_skid_mem(num_children, sizeof(int), &exit_code);
    // }
    // Parent's array of pollfd structs
    if (ENOERR == exit_code)
    {
        poll_fds = alloc_skid_mem(num_children, sizeof(struct pollfd), &exit_code);
    }
    // Parent's array of child PIDs
    if (ENOERR == exit_code)
    {
        child_pids = alloc_skid_mem(num_children, sizeof(pid_t), &exit_code);
    }

    // DO IT
    // Fork it
    if (ENOERR == exit_code)
    {
        for (unsigned int i = 0; i < num_children; i++)
        {
            // Create a temp, local pipe
            exit_code = create_pipes(&pipe_read_fd, &pipe_write_fd, O_NONBLOCK);
            if (ENOERR != exit_code)
            {
                PRINT_ERROR(The call to create_pipes() reported an error);
                PRINT_ERRNO(exit_code);
                break;  // We encountered an error, so stop looping
            }
            // Fork
            pid = fork();
            // Parent
            if (pid > 0)
            {
                // Store the PID
                child_pids[i] = pid;
                // Close the write end of the pipe
                close_pipe(&pipe_write_fd, false);
                // Add the read end of the pipe to the bookkeeping
                // fds[i] = pipe_read_fd;
                poll_fds[i].fd = pipe_read_fd;
                pipe_read_fd = SKID_BAD_FD;  // Reset the temp variable
            }
            // Child
            else if (0 == pid)
            {
                // Close the read end of the pipe
                close_pipe(&pipe_read_fd, false);
                // Start logging
                be_a_child(pipe_write_fd);
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
        while(ENOERR == exit_code)
        {
            if (SHUTDOWN_SIG == skid_sig_hand_signum)
            {
                fprintf(stdout, "\n%s is exiting\n", argv[0]);
                break;  // Time to cleanup and exit
            }
            // poll() here
            num_rdy = call_poll(poll_fds, num_children, -1, &exit_code);
            if (ENOERR != exit_code)
            {
                PRINT_ERROR(The call to call_poll() reported an error);
                PRINT_ERRNO(exit_code);
                break;  // We encountered an error, so stop looping
            }
            else if (0 == num_rdy)
            {
                printf("%s No children are ready yet\n", PARENT_STR);
                sleep(1);  // A tasteful sleep
            }
            else if (num_rdy > 0)
            {
                for (int i = 0; i < num_children; i++)
                {
                    if (SKID_BAD_FD != poll_fds[i].fd)
                    {
                        tmp_msg = read_pollfd(&(poll_fds[i]), &tmp_revents, &exit_code);
                        if (ENOERR == exit_code)
                        {
                            fprintf(stdout, "%s - %s\n", PARENT_STR, tmp_msg);
                            exit_code = free_skid_mem((void **)&tmp_msg);
                        }
                    }
                }
            }
        }
    }

    // CLEANUP
    // if (NULL != fds)
    // {
    //     free_skid_mem((void **)&fds);
    // }
    if (NULL != poll_fds)
    {
        free_skid_mem((void **)&poll_fds);
    }
    if (NULL != child_pids)
    {
        free_skid_mem((void **)&child_pids);
    }

    // DONE
    exit(exit_code);
}


void be_a_child(int fd)
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;       // Errno values
    unsigned int random_num = 0;  // Random sleep value
    char *tmp_msg = NULL;         // Temporary message

    // INPUT VALIDATION
    exit_code = validate_skid_fd(fd);

    // BE IT
    while (1)
    {
        // Randomize a number
        random_num = randomize_number(MAX_SLEEP, &exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to randomize_number() failed);
            PRINT_ERRNO(exit_code);
            break;
        }
        // Sleep
        sleep(random_num);
        // Time to shutdown?  Last chance...
        if (SHUTDOWN_SIG == skid_sig_hand_signum)
        {
            break;  // Time to cleanup and exit
        }
        // Generate a messge
        tmp_msg = generate_a_message(&exit_code);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to generate_a_message() failed);
            PRINT_ERRNO(exit_code);
            break;
        }
        // Write the message to the fd
        exit_code = write_fd(fd, tmp_msg);
        free_skid_mem((void **)&tmp_msg);  // Best effort, soonest
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to write_fd() failed);
            PRINT_ERRNO(exit_code);
            break;
        }
    }

    // CLEANUP
    close_pipe(&fd, false);  // Close the pipe file descriptor
    if (NULL != tmp_msg)
    {
        free_skid_mem((void **)&tmp_msg);  // Best effort
    }

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


char *generate_a_message(int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;     // Store errno value
    pid_t my_pid = getpid();  // This process' PID
    char *full_msg = NULL;    // Timestamped version of the base_msg
    // 20 for the largest PID, 1 for the nul character, and the rest for the string
    char base_msg[1024] = { "\0" };

    // INPUT VALIDATION
    results = validate_skid_err(errnum);

    // GENERATE IT
    if (ENOERR == results)
    {
        // Base message
        sprintf(base_msg, "Hello from PID %jd", (intmax_t)my_pid);
        // Timestamp it
        full_msg = timestamp_a_msg(base_msg, "[]", &results);
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return full_msg;
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
