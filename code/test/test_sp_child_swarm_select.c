/*
 *  This source file was created to demonstrate the proper usage of select().
 *      1. Sets up a "shutdown" signal handler
 *      2. Spawns a swarm of children, each with a dedicated unnamed pipe
 *      3. Maintains bookkeeping: PIDs, pipe fds, etc
 *      4. Each child will write to their unnamed pipe at psuedo-random intervals
 *      5. The parent will use select() to watch the read ends of the pipes
 *      6. It all gets shutdown when the signal is received
 *
 *  Copy/paste the following...

./code/dist/test_misc_child_swarm_select.bin <NUM_CHILDREN>

 *
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <fcntl.h>                          // O_NONBLOCK
#include <inttypes.h>                       // strtoumax()
#include <stdbool.h>                        // false
#include <stdio.h>                          // fprintf()
#include <stdlib.h>                         // exit()
#include <sys/wait.h>                       // waitpid()
#include <unistd.h>                         // fork()
#include "skid_debug.h"                     // MODULE_*LOAD(), *PRINT*_ERR*()
#include "skid_file_descriptors.h"          // read_fd(), write_fd()
#include "skid_macros.h"                    // ENOERR, SKID_BAD_FD, SKID_BAD_PID
#include "skid_memory.h"                    // free_skid_mem()
#include "skid_pipes.h"                     // close_pipe(), create_pipes()
#include "skid_random.h"                    // randomize_number()
#include "skid_select.h"                    // *_fd_set(), call_select()
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
 *  Everybody needs to be cleaning up so I made a SPOT.  All file descriptors in read_fds_ptr
 *  will be closed except for my_fd.  Pass SKID_BAD_FD in as my_fd to close all valid fds.
 *  Every call in this function will be quiet, as available, and "best effort".
 */
void clean_up(pid_t **child_pids_ptr, int **read_fds_ptr, unsigned int array_len,
              int my_fd);

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

/*
 *  Wait for all the children to die.
 */
void wait_for_children(pid_t *child_pids, int num_children, bool print);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;           // Errno values
    unsigned int num_children = 0;    // Number of children to spawn
    pid_t pid = SKID_BAD_PID;         // Temp return value from fork()
    int pipe_read_fd = SKID_BAD_FD;   // Temp read end of the pipe
    int pipe_write_fd = SKID_BAD_FD;  // Temp write end of the pipe
    pid_t *child_pids = NULL;         // Heap-allocated array for the parent to store child PIDs
    int *read_fds_arr = NULL;         // Heap-allocated array for the read file descriptors
    char *tmp_msg = NULL;             // Temp var w/ heap-allocated string read for select()ing fds
    int num_rdy = 0;                  // Number of fds ready
    int nfds = SKID_BAD_FD;           // Highest file descriptor + 1
    fd_set readfds;                   // In/Out parameter for the call to select()
    fd_set origfds;                   // Original set of read pipe file descriptors

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
        else if (num_children > FD_SETSIZE)
        {
            FPRINTF_ERR("%s select() structures do not suppor more than %d file descriptors\n",
                        DEBUG_ERROR_STR, FD_SETSIZE);
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
    // Parent's array of read pipe file descriptors
    if (ENOERR == exit_code)
    {
        read_fds_arr = alloc_skid_mem(num_children, sizeof(int), &exit_code);
    }
    // Parent's array of child PIDs
    if (ENOERR == exit_code)
    {
        child_pids = alloc_skid_mem(num_children, sizeof(pid_t), &exit_code);
    }
    // Initialize readfds
    if (ENOERR == exit_code)
    {
        exit_code = clear_fd_set(&readfds);
    }
    // Initialize origfds
    if (ENOERR == exit_code)
    {
        exit_code = clear_fd_set(&origfds);
    }

    // DO IT
    // Fork it
    if (ENOERR == exit_code)
    {
        for (unsigned int i = 0; i < num_children; i++)
        {
            // Create a temp, local pipe
            exit_code = create_pipes(&pipe_read_fd, &pipe_write_fd, O_NONBLOCK);
            // exit_code = create_pipes(&pipe_read_fd, &pipe_write_fd, 0);
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
                read_fds_arr[i] = pipe_read_fd;
                nfds = (pipe_read_fd > nfds) ? pipe_read_fd : nfds;
                pipe_read_fd = SKID_BAD_FD;  // Reset the temp variable
            }
            // Child
            else if (0 == pid)
            {
                // Close the read end of the pipe
                close_pipe(&pipe_read_fd, false);
                // Cleanup the parent process' allocations
                clean_up(&child_pids, &read_fds_arr, num_children, pipe_write_fd);
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

    // Start select()ing (in the parent)
    if (ENOERR == exit_code && pid > 0)
    {
        print_shutdown(argv[0]);
        // Setup fd_set
        exit_code = initialize_fd_set(read_fds_arr, num_children, &origfds);
        while(ENOERR == exit_code)
        {
            if (SHUTDOWN_SIG == skid_sig_hand_signum)
            {
                fprintf(stdout, "\n%s is exiting\n", argv[0]);
                break;  // Time to cleanup and exit
            }
            // Reset fd_set
            exit_code = copy_fd_set(&origfds, &readfds);
            if (ENOERR != exit_code)
            {
                PRINT_ERROR(The call to copy_fd_set() reported an error);
                PRINT_ERRNO(exit_code);
                break;  // We encountered an error so stop looping
            }
            // select() here
            num_rdy = call_select(nfds + 1, &readfds, NULL, NULL, NULL, &exit_code);  // Inf timeout
            if (ENOERR != exit_code)
            {
                if (EINTR != exit_code)
                {
                    PRINT_ERROR(The call to call_select() reported an error);
                    PRINT_ERRNO(exit_code);
                }
                break;  // We encountered an error (or a shutdown), so stop looping
            }
            else if (0 == num_rdy)
            {
                printf("%s: No children are ready yet\n", PARENT_STR);
            }
            else if (num_rdy > 0)
            {
                printf("%s: %d children are ready\n", PARENT_STR, num_rdy);
                for (int i = 0; i < num_children; i++)
                {
                    if (true == is_fd_in_set(read_fds_arr[i], &readfds, &exit_code) \
                        && ENOERR == exit_code)
                    {
                        tmp_msg = read_fd(read_fds_arr[i], &exit_code);
                        if (NULL != tmp_msg)
                        {
                            fprintf(stdout, "%s: %s\n", PARENT_STR, tmp_msg);
                            exit_code = free_skid_mem((void **)&tmp_msg);
                        }
                        else if (ENOERR != exit_code)
                        {
                            PRINT_ERROR(The call to read_fd() failed);
                            PRINT_ERRNO(exit_code);
                            break;  // Error encountered so stop looping
                        }
                    }
                    else if (ENOERR != exit_code)
                    {
                        PRINT_ERROR(The call to is_fd_in_set() reported an error);
                        PRINT_ERRNO(exit_code);
                    }
                }
            }
        }
    }

    // CLEANUP
    if (NULL != child_pids)
    {
        // Kill the children
        kill(0, SHUTDOWN_SIG);
        // Wait for the children to die
        wait_for_children(child_pids, num_children, true);  // print
    }
    // Free *everything*
    clean_up(&child_pids, &read_fds_arr, num_children, SKID_BAD_FD);  // Close them *all*

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


void clean_up(pid_t **child_pids_ptr, int **read_fds_ptr, unsigned int array_len,
              int my_fd)
{
    // LOCAL VARIABLES
    int *read_fds = NULL;  // Pointer to the array of file descriptors

    // CLEAN UP
    // Child PIDs array
    if (NULL != child_pids_ptr)
    {
        // Free the array of PIDs
        free_skid_mem((void **)child_pids_ptr);
    }
    // Array of read pipe file descriptors
    if (NULL != read_fds_ptr && NULL != *read_fds_ptr)
    {
        read_fds = *read_fds_ptr;
        for (unsigned int i = 0; i < array_len; i++)
        {
            if (my_fd != read_fds[i] && SKID_BAD_FD != read_fds[i])
            {
                close_pipe(read_fds + i, true);  // Best effort
            }
        }
        free_skid_mem((void **)read_fds_ptr);  // Best effort
    }
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
    fprintf(stdout, "%s has finished spawning children and is now select()ing pipes\n",
            prog_name);
    fprintf(stdout, "Terminate the server by sending signal [%d] %s\n",
            SHUTDOWN_SIG, strsignal(SHUTDOWN_SIG));
    fprintf(stdout, "E.g., kill -%d %jd\n", SHUTDOWN_SIG, (intmax_t)getpid());
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <NUM_CHILDREN>\n", prog_name);
}


void wait_for_children(pid_t *child_pids, int num_children, bool print)
{
    // LOCAL VARIABLES
    int results = ENOERR;          // Store errno value
    pid_t tmp_pid = SKID_BAD_PID;  // Return value from waitpid() call
    int tmp_status = 0;            // Out parameter for waitpid() call

    // INPUT VALIDATION
    if (NULL == child_pids || num_children < 1)
    {
        results = EINVAL;  // git gud
    }

    // WAIT
    for (int i = 0; i < num_children; i++)
    {
        tmp_pid = waitpid(child_pids[i], &tmp_status, 0);
        if (tmp_pid != child_pids[i])
        {
            results = errno;
            FPRINTF_ERR("%s The call to waitpid(%jd) failed\n", DEBUG_ERROR_STR,
                        (intmax_t)child_pids[i]);
            PRINT_ERRNO(results);
            continue;
        }

        if (true == print)
        {
            if (WIFEXITED(tmp_status))
            {
                printf("Child %jd exited with status %d\n", (intmax_t)tmp_pid,
                       WEXITSTATUS(tmp_status));
            }
            else if (WIFSIGNALED(tmp_status))
            {
                printf("Child %jd was terminated by signal %d\n", (intmax_t)tmp_pid,
                       WTERMSIG(tmp_status));
#ifdef WCOREDUMP
                if (WCOREDUMP(tmp_status))
                {
                    printf("Child %jd produced a core dump\n", (intmax_t)tmp_pid);
                }
#endif  /* WCOREDUMP */
            }
            else if (WIFSTOPPED(tmp_status))
            {
                printf("Child %jd was stopped by signal %d\n", (intmax_t)tmp_pid,
                       WSTOPSIG(tmp_status));
            }
            else if (WIFCONTINUED(tmp_status))
            {
                printf("Child %jd was continued\n", (intmax_t)tmp_pid);
            }
            else
            {
                printf("Child %jd changed state in an unknown way (status: 0x%x)\n",
                       (intmax_t)tmp_pid, tmp_status);
            }
        }
    }
}
