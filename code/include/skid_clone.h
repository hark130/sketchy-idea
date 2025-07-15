/*
 *  This library defines functionality to create a child process using clone().
 */

#define _GNU_SOURCE

#ifndef __SKID_CLONE__
#define __SKID_CLONE__

#include <linux/sched.h>                    // struct clone_args
#include <stdint.h>                         // uint64_t
#include "skid_macros.h"                    // ENOERR

/*
 *  Description:
 *      Call clone3(), by way of call_clone3_args(), after preparing the clone_args struct
 *      internally.  Automatically sets the exit_signal to SIGCHLD.
 *
 *  Args:
 *      flags: A bit-wise OR of zero or more flags (see: clone(2)).  Passed directly to
 *          clone3(), by way of clone_args.flags, without any real validation.
 *      stack: [Optional] Pointer to lowest byte of stack.  Set this to NULL64 (see: skid_macros.h)
 *          to use the same stack area as the parent (in the child's own virtual address space).
 *          If the CLONE_VM flag is used, this argument must be specified.
 *      stack_size: [Optional] Size of stack.  Set this to 0 to use the same stack area as the
 *          parent (in the child's own virtual address space).  If the CLONE_VM flag is used,
 *          this argument must be specified.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      The thread ID of the child process in the caller's thread of execution on success.
 *      0 is returned in the child process on success.  On error, SKID_BAD_PID error and no
 *      child process is created (see errnum value for details).
 */
pid_t call_clone3(uint64_t flags, uint64_t stack, uint64_t stack_size, int *errnum);

/*
 *  Description:
 *      Call clone3(), by way of syscall(), utilizing the provided clone_args struct.
 *
 *  Args:
 *      cl_args: Pointer to a user-prepared clone_args struct.  Passed directly to
 *          clone3() without any significant validation.
 *      size: The size of the structure pointed to by cl_args.  Passed directly to
 *          clone3() without any significant validation.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      The thread ID of the child process in the caller's thread of execution on success.
 *      0 is returned in the child process on success.  On error, SKID_BAD_PID error and no
 *      child process is created (see errnum value for details).
 */
pid_t call_clone3_args(const struct clone_args *cl_args, size_t size, int *errnum);

#endif  /* __SKID_CLONE__ */
