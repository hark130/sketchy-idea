/*
 *    This library defines functionality to help automate signal handling.
 */

// #define SKID_DEBUG                        // Enable DEBUG logging

#include "skid_debug.h"                      // PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"                // SignalHandler
#include "skid_signals.h"                // SignalHandler
#include "skid_signal_handlers.h"        // Externed atomic variables
#include <errno.h>                        // EINVAL
#include <stddef.h>                        // NULL
#include <sys/types.h>                    // pid_t
#include <sys/wait.h>                    // waitpid()

/**************************************************************************************************/
/*************************** SIGNAL HANDLER ATOMIC VARIABLE DEFINITION ****************************/
/**************************************************************************************************/

volatile sig_atomic_t skid_sig_hand_interrupted = 0;
volatile sig_atomic_t skid_sig_hand_data_int = 0;
volatile sig_atomic_t skid_sig_hand_data_ptr = 0;
volatile sig_atomic_t skid_sig_hand_ext = 0;
volatile sig_atomic_t skid_sig_hand_pid = 0;
volatile sig_atomic_t skid_sig_hand_queue = 0;
volatile sig_atomic_t skid_sig_hand_sigcode = 0;
volatile sig_atomic_t skid_sig_hand_signum = 0;
volatile sig_atomic_t skid_sig_hand_uid = 0;

/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *    Description:
 *        Block a signal.  The result will be the union of the current set and signum.
 *        It is not possible to block SIGKILL or SIGSTOP.  Attempts to do so are silently ignored.
 *        This function is reentrant and async-signal-safe.
 *
 *    Args:
 *        signum: The signal number.
 *
 *    Returns:
 *        On success, ENOERR is returned.  On error, errno is returned.
 */
int block_signal_safe(int signum);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/

/****************************** SA_HANDLER (SignalHandler) FUNCTIONS ******************************/


void handle_all_children(int signum)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int errnum = errno;  // Perserve the errno value
    pid_t result = 0;    // Result of waitpid

    // HANDLE IT
    do
    {
        result = waitpid(-1, NULL, WNOHANG);  // Wait for any child process
        if (result > 0)
        {
            // The PID of a child process that exited; Keep going
        }
        else if (0 == result)
        {
            // All child processes have exited
            break;
        }
        else if (-1 == result)
        {
            // An error occurred
        }
    } while (result > 0);

    // CLEANUP
    errno = errnum;  // Restore the original errno value
}


void handle_interruptions(int signum)
{
    if (SIGINT == signum)
    {
        skid_sig_hand_interrupted = 1;  // Handled SIGINT
    }
}


void handle_signal_number(int signum)
{
    skid_sig_hand_signum = signum;  // The signal number handled
}


/*************************** SA_SIGACTION (SignalHandlerExt) FUNCTIONS ****************************/


void handle_ext_read_queue_int(int signum, siginfo_t *info, void *context)
{
    // LOCAL VARIABLES
    union sigval data;              // Store the info->si_value here
    QueueData_t integer = Integer;  // skid_sig_hand_queue value

    // HANDLE IT
    if (NULL != info)
    {
        if (SI_QUEUE == info->si_code)
        {
            // Block further signals
            if (ENOERR == block_signal_safe(signum))
            {
                // Process the data
                data = info->si_value;
                skid_sig_hand_signum = info->si_signo;
                skid_sig_hand_sigcode = info->si_code;
                skid_sig_hand_pid = info->si_pid;
                skid_sig_hand_uid = info->si_uid;
                skid_sig_hand_data_int = data.sival_int;
                skid_sig_hand_queue = integer;
            }
        }
    }
}


void handle_ext_sending_process(int signum, siginfo_t *info, void *context)
{
    if (NULL != info)
    {
        if (SI_QUEUE == info->si_code || SI_USER == info->si_code)
        {
            skid_sig_hand_signum = info->si_signo;
            skid_sig_hand_sigcode = info->si_code;
            skid_sig_hand_pid = info->si_pid;
            skid_sig_hand_uid = info->si_uid;
            skid_sig_hand_ext = 1;
        }
    }
}


void handle_ext_signal_code(int signum, siginfo_t *info, void *context)
{
    if (NULL != info)
    {
        skid_sig_hand_signum = info->si_signo;
        skid_sig_hand_sigcode = info->si_code;
        skid_sig_hand_ext = 1;
    }
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


int block_signal_safe(int signum)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of the function call
    int errnum = errno;   // Preserve errno during this function call
    sigset_t block_mask;  // Pass this to sigprocmask()

    // BLOCK IT
    if (sigemptyset(&block_mask))
    {
        result = errno;
    }
    else if (sigaddset(&block_mask, SIGUSR1))
    {
        result = errno;
    }
    else if (sigprocmask(SIG_BLOCK, &block_mask, NULL))
    {
        result = errno;
    }

    // DONE
    errno = errnum;  // Restore errno
    return result;
}
