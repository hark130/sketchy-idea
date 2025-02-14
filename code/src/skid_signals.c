/*
 *	This library defines functionality to help automate signal handling.
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE					// Expose the TRAP_* signal code macros
#endif  /* _XOPEN_SOURCE */
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED			// Expose the TRAP_* signal code macros
#endif  /* _XOPEN_SOURCE_EXTENDED */

// #define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"				// NOERR
#include "skid_memory.h"				// copy_skid_string()
#include "skid_signals.h"				// Signal MACROs, SignalHandler
#include "skid_validation.h"			// validate_skid_err()
#include <errno.h>						// EINVAL
#include <string.h>						// memset()
#include <sys/types.h>					// pid_t
#include <sys/wait.h>					// waitpid()


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *	Description:
 *		Call sigaction() and handle errors in a standardized way.  Input is not validated.
 *
 *	Args:
 *		signum: The signal number.
 *		newact: Pointer to a new sigaction struct.
 *		oldact: [Optional] Pointer to a storage location for the old sigaction struct.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int call_sigaction(int signum, struct sigaction *newact, struct sigaction *oldact);

/*
 *	Description:
 *		Call sigprocmas() and handle errors in a standardized way.  Input is not validated.
 *
 *	Args:
 *		how: Controls the behavior of sigprocmask() (see: signprocmask(2)).
 *		set: Pointer to a new signal set.
 *		oldset: [Optional] Pointer to a storage location for the previous set.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int call_sigprocmask(int how, sigset_t *set, sigset_t *oldset);

/*
 *	Description:
 *		Initialize action's memory and empty the signal set.
 *
 *	Args:
 *		action: The signal set to initialize.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int initialize_sigaction_struct(struct sigaction *action);

/*
 *	Description:
 *		Empty this signal set and add signum to it.
 *
 *	Notes:
 *		Use sigemptyset() (see: sigsetops(3)) to initialize a signal set without adding a signal.
 *
 *	Args:
 *		set: The signal set to initialize.
 *		signum: The signal number to add.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int initialize_signal_set(sigset_t *set, int signum);

/*
 *	Description:
 *		Translate signal codes for generic signals into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_generic(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGBUS signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigbus(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGCHLD signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigchld(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGSEGV signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigsegv(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGFPE signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigfpe(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGILL signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigill(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGIO signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigio(int signum, int sigcode, int *errnum);

/*
 *	Description:
 *		Translate SIGTRAP signal codes into a human-readable, heap-allocated string.
 *
 *	Args:
 *		signum: The signal number.
 *		sigcode: The signal code.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated string describing the signal code with relation to the signal number.
 *		NULL on error (check errnum for details).
 */
char *translate_signal_sigtrap(int signum, int sigcode, int *errnum);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int block_signal(int signum, sigset_t *oldset)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value
	int how = SIG_BLOCK;  // Tells sigprocmask() to block signals
	sigset_t set;    	  // New signal set (intialized later)

	// BLOCK IT
	// Initialize the set
	result = initialize_signal_set(&set, signum);
	// Call sigprocmask
	if (ENOERR == result)
	{
		result = call_sigprocmask(how, &set, oldset);
	}

	// DONE
	return result;
}


int set_signal_handler(int signum, SignalHandler handler, int flags, struct sigaction *oldact)
{
	// LOCAL VARIABLES
	int result = ENOERR;      // Errno value
	struct sigaction newact;  // Argument for sigaction()

	// INPUT VALIDATION
	if (NULL == handler)
	{
		result = EINVAL;  // NULL pointer
	}

	// SETUP
	// Initialize the struct
	if (ENOERR == result)
	{
		result = initialize_sigaction_struct(&newact);
	}
	// Assign the struct values
	if (ENOERR == result)
	{
		newact.sa_handler = handler;
		newact.sa_flags = flags;
	}

	// SET IT
	if (ENOERR == result)
	{
		result = call_sigaction(signum, &newact, oldact);
	}

	// DONE
	return result;
}


int set_signal_handler_ext(int signum, SignalHandlerExt handler, int flags,
	                       struct sigaction *oldact)
{
	// LOCAL VARIABLES
	int result = ENOERR;      // Errno value
	struct sigaction newact;  // Argument for sigaction()

	// INPUT VALIDATION
	if (NULL == handler)
	{
		result = EINVAL;  // NULL pointer
	}

	// SETUP
	// Initialize the struct
	if (ENOERR == result)
	{
		result = initialize_sigaction_struct(&newact);
	}
	// Assign the struct values
	if (ENOERR == result)
	{
		newact.sa_sigaction = handler;
		newact.sa_flags = flags | SA_SIGINFO;  // Explicitly adds the flag to use sa_sigaction
	}

	// SET IT
	if (ENOERR == result)
	{
		result = call_sigaction(signum, &newact, oldact);
	}

	// DONE
	return result;
}


char *translate_signal_code(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (signum)
		{
			case SIGILL:
				translation = translate_signal_sigill(signum, sigcode, errnum);
				break;
			case SIGFPE:
				translation = translate_signal_sigfpe(signum, sigcode, errnum);
				break;
		    case SIGSEGV:
				translation = translate_signal_sigsegv(signum, sigcode, errnum);
				break;
		    case SIGBUS:
				translation = translate_signal_sigbus(signum, sigcode, errnum);
				break;
		    case SIGTRAP:
				translation = translate_signal_sigtrap(signum, sigcode, errnum);
				break;
		    case SIGCHLD:
				translation = translate_signal_sigchld(signum, sigcode, errnum);
				break;
		    case SIGIO:  // Also SIGPOLL, apparently
				translation = translate_signal_sigio(signum, sigcode, errnum);
				break;
			default:
				translation = translate_signal_generic(signum, sigcode, errnum);
		}
	}

	// DONE
	return translation;
}


int unblock_signal(int signum, sigset_t *oldset)
{
	// LOCAL VARIABLES
	int result = ENOERR;    // Errno value
	int how = SIG_UNBLOCK;  // Tells sigprocmask() to unblock signals
	sigset_t set;    	    // New signal set (intialized later)

	// BLOCK IT
	// Initialize the set
	result = initialize_signal_set(&set, signum);
	// Call sigprocmask
	if (ENOERR == result)
	{
		result = call_sigprocmask(how, &set, oldset);
	}

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/

int call_sigaction(int signum, struct sigaction *newact, struct sigaction *oldact)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// CALL IT
	if (sigaction(signum, newact, oldact))
	{
		result = errno;
		PRINT_ERROR(The call to sigaction() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int call_sigaddset(sigset_t *set, int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (NULL == set)
	{
		result = EINVAL;  // NULL pointer
	}

	// CALL IT
	if (ENOERR == result)
	{
		if(sigaddset(set, signum))
		{
			result = errno;
			PRINT_ERROR(The call to sigaddset() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int call_sigemptyset(sigset_t *set)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// CALL IT
	if (sigemptyset(set))
	{
		result = errno;
		PRINT_ERROR(The call to sigaction() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int call_sigprocmask(int how, sigset_t *set, sigset_t *oldset)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// CALL IT
	if (sigprocmask(how, set, oldset))
	{
		result = errno;
		PRINT_ERROR(The call to sigprocmask() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int initialize_sigaction_struct(struct sigaction *action)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (NULL == action)
	{
		result = EINVAL;  // NULL pointer
	}

	// INITIALIZE IT
	if (ENOERR == result)
	{
		// Clear the struct
		memset(action, 0x0, sizeof(struct sigaction));
		// Exclude all the signals from the set
		if (sigemptyset(&(action->sa_mask)))
		{
			result = errno;
			PRINT_ERROR(The call to sigemptyset() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}

int initialize_signal_set(sigset_t *set, int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (NULL == set)
	{
		result = EINVAL;  // NULL pointer
	}

	// INITIALIZE IT
	// Empty it
	if (ENOERR == result)
	{
		result = call_sigemptyset(set);
	}
	// Add signum to the set
	if (ENOERR == result)
	{
		result = call_sigaddset(set, signum);
	}

	// DONE
	return result;
}


char *translate_signal_generic(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
		    case SI_ASYNCIO:
				temp_str = "SI_ASYNCIO: AIO completed";
				break;
			case SI_KERNEL:
				temp_str = "SI_KERNEL: Sent by the kernel";
				break;
		    case SI_MESGQ:
				temp_str = "SI_MESGQ: POSIX message queue state changed; see mq_notify(3)";
				break;
		    case SI_QUEUE:
				temp_str = "SI_QUEUE: See sigqueue(3)";
				break;
		    case SI_SIGIO:
				temp_str = "SI_SIGIO: Queued SIGIO (from a legacy kernel version)";
				break;
		    case SI_TIMER:
				temp_str = "SI_TIMER: POSIX timer expired";
				break;
		    case SI_TKILL:
				temp_str = "SI_TKILL: tkill(2) or tgkill(2)";
				break;
			case SI_USER:
				temp_str = "SI_USER: kill command";
				break;
			default:
				temp_str = "UNKNOWN SIGNAL CODE";
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		translation = copy_skid_string(temp_str, &result);
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigbus(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGBUS != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case BUS_ADRALN:
				temp_str = "BUS_ADRALN: Invalid address alignment";
				break;
			case BUS_ADRERR:
				temp_str = "BUS_ADRERR: Nonexistent physical address";
				break;
			case BUS_MCEERR_AO:
				temp_str = "BUS_MCEERR_AO: Hardware memory error detected in process but not \
				            consumed; action optional";
				break;
			case BUS_MCEERR_AR:
				temp_str = "BUS_MCEERR_AR: Hardware memory error consumed on a machine check; \
				            action required";
				break;
			case BUS_OBJERR:
				temp_str = "BUS_OBJERR: Object-specific hardware error";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigchld(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGCHLD != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case CLD_CONTINUED:
				temp_str = "CLD_CONTINUED: Stopped child has continued";
				break;
			case CLD_DUMPED:
				temp_str = "CLD_DUMPED: Child terminated abnormally";
				break;
			case CLD_EXITED:
				temp_str = "CLD_EXITED: Child has exited";
				break;
			case CLD_KILLED:
				temp_str = "CLD_KILLED: Child was killed";
				break;
			case CLD_STOPPED:
				temp_str = "CLD_STOPPED: Child has stopped";
				break;
			case CLD_TRAPPED:
				temp_str = "CLD_TRAPPED: Traced child has trapped";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigsegv(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGSEGV != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case SEGV_ACCERR:
				temp_str = "SEGV_ACCERR: Invalid permissions for mapped object";
				break;
			case SEGV_MAPERR:
				temp_str = "SEGV_MAPERR: Address not mapped to object";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigfpe(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGFPE != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case FPE_FLTDIV:
				temp_str = "FPE_FLTDIV: Floating-point divide by zero";
				break;
			case FPE_FLTINV:
				temp_str = "FPE_FLTINV: Floating-point invalid operation";
				break;
			case FPE_FLTOVF:
				temp_str = "FPE_FLTOVF: Floating-point overflow";
				break;
			case FPE_FLTRES:
				temp_str = "FPE_FLTRES: Floating-point inexact result";
				break;
			case FPE_FLTUND:
				temp_str = "FPE_FLTUND: Floating-point underflow";
				break;
			case FPE_INTDIV:
				temp_str = "FPE_INTDIV: Integer divide by zero";
				break;
			case FPE_INTOVF:
				temp_str = "FPE_INTOVF: Integer overflow";
				break;
			case FPE_FLTSUB:
				temp_str = "FPE_FLTSUB: Subscript out of range";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigill(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGILL != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case ILL_BADSTK:
				temp_str = "ILL_BADSTK: Internal stack error";
				break;
			case ILL_COPROC:
				temp_str = "ILL_COPROC: Coprocessor error";
				break;
			case ILL_ILLADR:
				temp_str = "ILL_ILLADR: Illegal addressing mode";
				break;
			case ILL_ILLOPC:
				temp_str = "ILL_ILLOPC: Illegal opcode";
				break;
			case ILL_ILLOPN:
				temp_str = "ILL_ILLOPN: Illegal operand";
				break;
			case ILL_ILLTRP:
				temp_str = "ILL_ILLTRP: Illegal trap";
				break;
			case ILL_PRVOPC:
				temp_str = "ILL_PRVOPC: Privileged opcode";
				break;
			case ILL_PRVREG:
				temp_str = "ILL_PRVREG: Privileged register";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigio(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGIO != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case POLL_ERR:
				temp_str = "POLL_ERR: I/O error";
				break;
			case POLL_HUP:
				temp_str = "POLL_HUP: Device disconnected";
				break;
			case POLL_IN:
				temp_str = "POLL_IN: Data input available";
				break;
			case POLL_MSG:
				temp_str = "POLL_MSG: Input message available";
				break;
			case POLL_OUT:
				temp_str = "POLL_OUT: Output buffers available";
				break;
			case POLL_PRI:
				temp_str = "POLL_PRI: High priority input available";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}


char *translate_signal_sigtrap(int signum, int sigcode, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;       // Errno value
	char *temp_str = NULL;     // Temporary string to copy into translation
	char *translation = NULL;  // Human readable signal code translation

	// VALIDATION
	result = validate_skid_err(errnum);
	if (ENOERR == result)
	{
		if (SIGTRAP != signum)
		{
			result = EINVAL;  // Wrong signal
		}
	}

	// TRANSLATE IT
	if (ENOERR == result)
	{
		switch (sigcode)
		{
			case TRAP_BRANCH:
				temp_str = "TRAP_BRANCH: Process taken branch trap";
				break;
			case TRAP_BRKPT:
				temp_str = "TRAP_BRKPT: Process breakpoint";
				break;
			case TRAP_HWBKPT:
				temp_str = "TRAP_HWBKPT: Hardware breakpoint/watchpoint";
				break;
			case TRAP_TRACE:
				temp_str = "TRAP_TRACE: Process trace trap";
				break;
		}
	}
	// COPY IT
	if (ENOERR == result)
	{
		// Do we need to find a generic destription as a fallback?
		if (NULL == temp_str)
		{
			translation = translate_signal_generic(signum, sigcode, &result);
		}
		else
		{
			translation = copy_skid_string(temp_str, &result);
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return translation;
}
