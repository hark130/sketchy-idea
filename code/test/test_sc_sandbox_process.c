/*
 *  Manually test skid_clone.h's clone3 functionality.
 *
 *  This manual test code utilizes clone3 to sandbox a child process.
 *
 *  Copy/paste the following...

./code/dist/test_sc_sandbox_process.bin

 *
 */

// Standard includes
#include <errno.h>                          // EINVAL
// #include <stdbool.h>                  // bool, false, true
#include <stdint.h>                         // uint64_t
// #include <stdio.h>                    // fprintf(), printf()
// #include <stdlib.h>                   // exit()
#include <sys/wait.h>                       // waitpid()
// #include <unistd.h>                   // fork()
// Local includes
#define SKID_DEBUG                          // The DEBUG output is doing double duty as test output
#include "skid_clone.h"                     // call_clone3()
#include "skid_debug.h"                     // DEBUG_* macros, PRINT_*()
#include "skid_macros.h"                    // ENOERR

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


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Store errno and/or results here
    pid_t the_pid = 0;    // Return value from call_clone3()
    // Flags to pass to clone3
    int flags = CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | CLONE_SIGHAND | CLONE_THREAD;

    // INPUT VALIDATION
    if (argc != 1)
    {
        print_usage(argv[0]);
        result = EINVAL;
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


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s\n", prog_name);
}


int run_the_child(void)
{
    // LOCAL VARIABLES
    int result = ENOERR;   // Store errno and/or results here

    // RUN IT
    printf("%s CHILD - I'm also here\n", DEBUG_INFO_STR);  // DEBUGGING

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

    // RUN IT
    printf("%s PARENT - I'm here\n", DEBUG_INFO_STR);  // DEBUGGING

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
            PRINT_ERROR(PARENT - The waitpid() call failed);
            result = child_status;
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

    // DONE
    return result;
}
