/*
 *  Manually test skid_clone.h's clone3 functionality.
 *
 *  This manual test code utilizes clone3 flags to sandbox a child process.
 *
 *  Copy/paste the following...

sudo ./code/dist/test_sc_sandbox_process.bin            # Free to wreck havoc
sudo ./code/dist/test_sc_sandbox_process.bin --sandbox  # Sandbox!

 *
 */

// Standard includes
#include <errno.h>                          // EINVAL
#include <limits.h>                         // HOST_NAME_MAX
#include <inttypes.h>                       // PRIdMAX
#include <stdbool.h>                        // bool, false, true
#include <stdint.h>                         // uint64_t
#include <stdio.h>                          // fprintf(), printf()
#include <string.h>                         // strncmp()
#include <stdlib.h>                         // exit()
#include <sys/wait.h>                       // waitpid()
#include <unistd.h>                         // gethostname(), sethostname(), sleep()
// Local includes
#define SKID_DEBUG                          // The DEBUG output is doing double duty as test output
#include "skid_clone.h"                     // call_clone3()
#include "skid_debug.h"                     // DEBUG_* macros, PRINT_*()
#include "skid_macros.h"                    // ENOERR

#define CLI_ARG "--sandbox"                 // Command line argument to invoke a sandbox
#define MY_HOSTNAME "LDP-CDE-v0"            // Manual testing has seen this get changed
#define NEW_HOSTNAME "sandbox"              // New hostname
#define MAIN_NAME "MAIN()"                  // Allow the main() function to identify itself
#define PARENT_NAME "PARENT"                // Allow the parent process to identify itself
#define CHILD_NAME "CHILD"                  // Allow the child process to identify itself


/*
 *  Call gethostname(), prints the results to stdout, and respond to errors.
 */
int print_hostname(const char *whoami);

/*
 *  Print the PID of the process' parent.
 */
void print_parent_pid(const char *whoami);

/*
 *  Print the process' PID.
 */
void print_pid(const char *whoami);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Be the sandboxed child process.
 */
int run_the_child(void);

/*
 *  Be the parent process.
 */
int run_the_parent(pid_t child_pid);

/*
 *  Call sethostname() and respond to errors.
 */
int set_hostname(const char *whoami, const char *name, size_t len);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int result = ENOERR;                   // Store errno and/or results here
    pid_t the_pid = 0;                     // Return value from call_clone3()
    int flags = 0;                         // Flags to pass to clone3
    char cli_arg[] = { CLI_ARG };          // Watch for this CLI argument
    size_t cli_arg_len = strlen(cli_arg);  // Length of the CLI argument
    // Pass these sandbox flags to clone3 *if* --sandbox was passed
    // CLONE_NEWPID masks original PIDs by creating the process in a new PID namespace
    // CLONE_NEWUTS protects the hostname (among other thigs) by using a new UTS namespace
    int sand_flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWUTS;  // New everything!

    // INPUT VALIDATION
    if (argc == 2)
    {
        if (strlen(argv[1]) != cli_arg_len \
            || 0 != strncmp(argv[1], cli_arg, cli_arg_len * sizeof(char)))
        {
            fprintf(stderr, "Invalid command line argument: %s", argv[1]);
            print_usage(argv[0]);
            result = EINVAL;
        }
        else
        {
            printf("%s %s - Using sandbox flags with clone3\n", DEBUG_INFO_STR, MAIN_NAME);
            flags = sand_flags;
        }
    }
    else if (argc != 1)
    {
        print_usage(argv[0]);
        result = EINVAL;
    }

    // SETUP
    if (ENOERR == result)
    {
        print_pid(MAIN_NAME);  // Print main()'s PID
        result = set_hostname(MAIN_NAME, "LDP-CDE-v0", 10);  // Reset the hostname
    }

    // SANDBOX IT
    // Call clone3
    if (ENOERR == result)
    {
        the_pid = call_clone3(flags, NULL64, 0, &result);
        if (SKID_BAD_PID == the_pid)
        {
            PRINT_ERROR(The call_clone3() call failed);
            PRINT_ERRNO(result);
        }
    }

    // Respond
    // Parent
    if (ENOERR == result)
    {
        if (the_pid > 0)
        {
            result = run_the_parent(the_pid);
        }
    }

    // Child
    if (ENOERR == result)
    {
        if (0 == the_pid)
        {
            result = run_the_child();
        }
    }

    // DONE
    return result;
}


int print_hostname(const char *whoami)
{
    // LOCAL VARIABLES
    int result = ENOERR;                          // Store errno and/or results here
    char hostname[HOST_NAME_MAX + 1] = { "\0" };  // Store the hostname here

    // PRINT IT
    if (-1 == gethostname(hostname, HOST_NAME_MAX * sizeof(hostname[0])))
    {
        result = errno;
        FPRINTF_ERR("%s %s - The gethostname() call failed", DEBUG_ERROR_STR, whoami);
        PRINT_ERRNO(result);
    }
    else
    {
        printf("%s I am the %s and my hostname is %s\n", DEBUG_INFO_STR, whoami, hostname);
    }

    // DONE
    return result;
}


void print_parent_pid(const char *whoami)
{
    printf("%s I am the %s and I think my parent's PID is %" PRIdMAX "\n", DEBUG_INFO_STR, whoami,
           (intmax_t)getppid());
}


void print_pid(const char *whoami)
{
    printf("%s I am the %s and I think my PID is %" PRIdMAX "\n", DEBUG_INFO_STR, whoami,
           (intmax_t)getpid());
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s [%s]\n", prog_name, CLI_ARG);
}


int run_the_child(void)
{
    // LOCAL VARIABLES
    int result = ENOERR;                     // Store errno and/or results here
    char new_hostname[] = { NEW_HOSTNAME };  // New hostname

    // RUN IT
    // Do some things
    if (ENOERR == result)
    {
        print_pid(CHILD_NAME);
        print_parent_pid(CHILD_NAME);
        result = set_hostname(CHILD_NAME, new_hostname,
                              sizeof(new_hostname) / sizeof(new_hostname[0]));
    }
    // Print the hostname
    if (ENOERR == result)
    {
        result = print_hostname(CHILD_NAME);
    }

    // DONE
    return result;
}


int run_the_parent(pid_t child_pid)
{
    // LOCAL VARIABLES
    int result = ENOERR;   // Store errno and/or results here
    pid_t wait_ret = 0;    // Return value from the call to waitpid()
    int child_status = 0;  // Status information about the child process

    // INPUT VALIDATION
    if (SKID_BAD_PID == child_pid)
    {
        result = EINVAL;  // Bad PID
    }
    else
    {
        printf("%s I am the %s and I have cloned a child process with PID %" PRIdMAX "\n",
               DEBUG_INFO_STR, PARENT_NAME, (intmax_t)child_pid);
    }

    // RUN IT
    // Wait for the child to finish
    while (ENOERR == result)
    {
        // Check the child process
        wait_ret = waitpid(child_pid, &child_status, WNOHANG);  // Is the child alive?
        if (child_pid == wait_ret)
        {
            // The child's state has changed
            if (WIFEXITED(child_status) || WIFSIGNALED(child_status) || WIFSTOPPED(child_status))
            {
                // The child is no more
                printf("%s PARENT - The child has finished\n", DEBUG_INFO_STR);
                break;
            }
        }
        else if (0 == wait_ret)
        {
            // The child didn't change state yet... be patient
        }
        else if (-1 == wait_ret)
        {
            result = errno;
            PRINT_ERROR(PARENT - The waitpid() call failed);
            if (ENOERR == result)
            {
                result = child_status;
            }
            PRINT_ERRNO(result);
            break;
        }
        else
        {
            PRINT_ERROR(PARENT - The call to waitpid() reported an unknown PID);
            FPRINTF_ERR("PARENT - The call to waitpid() reported an unknown PID: %d\n",
                        wait_ret);
            break;
        }
    }
    // Print the hostname
    if (ENOERR == result)
    {
        result = print_hostname(PARENT_NAME);
    }

    // DONE
    return result;
}


int set_hostname(const char *whoami, const char *name, size_t len)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Store errno and/or results here

    // SET IT
    if (-1 == sethostname(name, len))
    {
        result = errno;
        FPRINTF_ERR("%s %s - The sethostname() call failed", DEBUG_ERROR_STR, whoami);
        PRINT_ERRNO(result);
    }
    else
    {
        printf("%s I am the %s and I have changed my hostname to %s\n",
               DEBUG_INFO_STR, whoami, name);
    }

    // DONE
    return result;
}
