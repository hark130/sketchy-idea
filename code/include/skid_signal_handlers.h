#ifndef __SKID_SIGNAL_HANDLERS__
#define __SKID_SIGNAL_HANDLERS__

#include <signal.h>                     // siginfo_t

/*
 *    Description:
 *        Function pointer to be used with struct sigaction's sa_handler member.
 *        This function is reentrant and async-signal-safe.
 *
 *    Args:
 *        signum: The signal number.
 */
typedef void (*SignalHandler)(int signum);

/*
 *    Description:
 *        Function pointer to be used with struct sigaction's sa_sigaction member.
 *        This function is reentrant and async-signal-safe.
 *
 *    Args:
 *        signum: The signal number.
 *        info: A pointer to a structure used to provide additional information about the signal.
 *        context: A pointer to a structure of type ucontext_t (defined in <ucontext.h>) which
 *            allegedly provides user context information describing the process state prior to
 *            invocation of the signal handler, including the previous signal mask and saved
 *            register values.
 */
typedef void (*SignalHandlerExt)(int signum, siginfo_t *info, void *context);

/*
 *    Description:
 *        Used by some extended signal handlers to indicate which atomic variable to read data from.
 *        Avoid using zero (0) as a value since all atomic signal handler variables are initialized
 *        to that value.
 */
typedef enum { Integer = 1, Pointer = 2 } QueueData_t;

/**************************************************************************************************/
/*************************** SIGNAL HANDLER ATOMIC VARIABLE DECLARATION ***************************/
/**************************************************************************************************/

// NOTES:
//    - Do *NOT* define these variables in your code.  They are defined in skid_signal_handlers.c.
//    - All flags are initialized to zero (0).
//    - Only use the assignment operator on these flags, or retrieve values, to ensure atomic usage.
//    - Signal handlers in this library indicate some/all/none flags utilized, per handler.

extern volatile sig_atomic_t skid_sig_hand_interrupted;  // Non-zero values indicate SIGINT handled
extern volatile sig_atomic_t skid_sig_hand_data_int;     // Data (int)
extern volatile void *skid_sig_hand_data_ptr;            // Data (void*)
extern volatile sig_atomic_t skid_sig_hand_ext;          // Non-zero values indicate ext reporting
extern volatile sig_atomic_t skid_sig_hand_pid;          // PID of a sending process
extern volatile sig_atomic_t skid_sig_hand_queue;        // QueueData_t indicates queue data
extern volatile sig_atomic_t skid_sig_hand_sigcode;      // siginfo_t.si_code value (signal code)
extern volatile sig_atomic_t skid_sig_hand_signum;       // Non-zero values indicate the signal num
extern volatile sig_atomic_t skid_sig_hand_uid;          // Real UID of a sending process

/**************************************************************************************************/
/****************************** SA_HANDLER (SignalHandler) FUNCTIONS ******************************/
/**************************************************************************************************/

/*
 *    Description:
 *        This signal handler waits for all child processes to exit without hanging by calling:
 *            waitpid(-1, NULL, WNOHANG)
 */
void handle_all_children(int signum);

/*
 *    Description:
 *        This handler sets the skid_sig_hand_interrupted atomic variable when SIGINT is handled.
 */
void handle_interruptions(int signum);

/*
 *    Description:
 *        This signal handler sets the skid_sig_hand_signum atomic variable when a signal is handled.
 */
void handle_signal_number(int signum);

/**************************************************************************************************/
/*************************** SA_SIGACTION (SignalHandlerExt) FUNCTIONS ****************************/
/**************************************************************************************************/

/*
 *    Description:
 *        This extended signal handler will read integer data sent for signum via sigqueue().
 *        It sets the skid_sig_hand_queue atomic variable when it is ready to report.  The value
 *        indicates the type of data to be read.  Interpret that value as type QueueData_t.
 *        If skid_sig_hand_queue is QueueData_t.Integer, retrieve values from:
 *            - skid_sig_hand_data_int
 *            - skid_sig_hand_pid
 *            - skid_sig_hand_sigcode (SI_QUEUE)
 *            - skid_sig_hand_signum
 *            - skid_sig_hand_uid
 *        This signal handler will not clear its own flags, merely overwrite them.  The user
 *        should clear the flags if execution is to continue.  Also, this extended signal handler
 *        will block further signum signals in an attempt to give the user a chance to process
 *        the data before the next signal is handled.  The user must unblock signum after processing.
 */
void handle_ext_read_queue_int(int signum, siginfo_t *info, void *context);

/*
 *    Description:
 *        This extended signal handler will read a pointer sent for signum via sigqueue().
 *        It sets the skid_sig_hand_queue atomic variable when it is ready to report.  The value
 *        indicates the type of data to be read.  Interpret that value as type QueueData_t.
 *        If skid_sig_hand_queue is QueueData_t.Pointer, retrieve values from:
 *            - skid_sig_hand_data_ptr
 *            - skid_sig_hand_pid
 *            - skid_sig_hand_sigcode (SI_QUEUE)
 *            - skid_sig_hand_signum
 *            - skid_sig_hand_uid
 *        This signal handler will not clear its own flags, merely overwrite them.  The user
 *        should clear the flags if execution is to continue.  Also, this extended signal handler
 *        will block further signum signals in an attempt to give the user a chance to process
 *        the data before the next signal is handled.  The user must unblock signum after processing.
 */
void handle_ext_read_queue_ptr(int signum, siginfo_t *info, void *context);

/*
 *    Description:
 *        This extended signal handler provides the PID and UID of the sending process for signals
 *        sent via kill() or sigqueue().
 *        It sets the skid_sig_hand_ext atomic variable when it is ready to report.
 *        If skid_sig_hand_ext is non-zero, retrieve values from:
 *            - skid_sig_hand_pid
 *            - skid_sig_hand_sigcode (SI_QUEUE or SI_USER)
 *            - skid_sig_hand_signum
 *            - skid_sig_hand_uid
 *        This signal handler will not clear its own flags, merely overwrite them.  The user
 *        should clear the flags if execution is to continue.
 */
void handle_ext_sending_process(int signum, siginfo_t *info, void *context);

/*
 *    Description:
 *        This signal handler will provide the signal number and the signal code.
 *        It sets the skid_sig_hand_ext atomic variable when it is ready to report.
 *        If skid_sig_hand_ext is non-zero, retrieve values from:
 *            - skid_sig_hand_sigcode
 *            - skid_sig_hand_signum
 *        This signal handler will not clear its own flags, merely overwrite them.  The user
 *        should clear the flags if execution is to continue.
 */
void handle_ext_signal_code(int signum, siginfo_t *info, void *context);

#endif  /* __SKID_SIGNAL_HANDLERS__ */
