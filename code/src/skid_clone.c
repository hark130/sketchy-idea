/*
 *  This library defines functionality to create a child process using clone().
 */

#define _GNU_SOURCE

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL, errno
#include <linux/sched.h>                    // struct clone_args
#include <signal.h>                         // SIGCHLD
#include <string.h>                         // memset()
#include <sys/syscall.h>                    // SYS_clone3
#include <unistd.h>                         // syscall()
#include "skid_clone.h"                     // call_clone3_args()
#include "skid_debug.h"                     // MODULE_*LOAD(), *PRINT_*()
#include "skid_macros.h"                    // ENOERR, NULL64, SKID_BAD_PID
#include "skid_validation.h"                // validate_skid_err()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute

#if defined(SYS_clone3) && __GLIBC__ >= 2 && defined(CLONE_NEWUTS)
/* Architecture supports clone3 syscall */
#else
#error "clone3 syscall unavailable: update kernel/glibc"
#endif  /* clone3 syscall test */

/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/



/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


pid_t call_clone3(uint64_t flags, uint64_t stack, uint64_t stack_size, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_skid_err(errnum);  // Results of execution
    pid_t pid = SKID_BAD_PID;                // PID returned by the clone3 syscall
    struct clone_args cl_args;               // Pass arguments into the clone3 system call
    size_t cl_size = sizeof(cl_args);        // A required value for the clone3 system call

    // INPUT VALIDATION
    if (ENOERR == result)
    {
        // Either both should be defined or neither should be defined
        if (NULL64 == stack && 0 != stack_size)
        {
            result = EINVAL;
        }
        else if (NULL64 != stack && 0 == stack_size)
        {
            result = EINVAL;
        }
    }

    // CALL IT
    if (ENOERR == result)
    {
        memset(&cl_args, 0x0, cl_size);  // Zeroize the struct
        cl_args.flags = flags;
        cl_args.exit_signal = SIGCHLD;  // Signal to deliver to parent on child termination
        cl_args.stack = stack;
        cl_args.stack_size = stack_size;
        pid = call_clone3_args(&cl_args, cl_size, &result);
        if (SKID_BAD_PID == pid)
        {
            PRINT_ERROR(The call to call_clone3_args() returned a bad PID);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return pid;
}


pid_t call_clone3_args(const struct clone_args *cl_args, size_t size, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_skid_err(errnum);  // Results of execution
    pid_t pid = SKID_BAD_PID;                // PID returned by the clone3 syscall

    // INPUT VALIDATION
    if (ENOERR == result)
    {
        if (NULL == cl_args || 0 >= size)
        {
            result = EINVAL;
        }
    }

    // CALL IT
    if (ENOERR == result)
    {
        pid = syscall(SYS_clone3, cl_args, size);
        if (pid < 0)
        {
            result = errno;
            PRINT_ERROR(The syscall to clone3 failed);
            PRINT_ERRNO(result);
            pid = SKID_BAD_PID;  // Standardize what a "bad PID" is
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return pid;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
