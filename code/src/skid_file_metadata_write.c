/*
 *	This library defines functionality to modify Linux file metadata.
 */

#define _POSIX_C_SOURCE 200809L		  // Expose utimensat()
// #define SKID_DEBUG                    // Enable DEBUGGING output

#include <fcntl.h>					  // AT_FDCWD
#include "skid_debug.h"				  // PRINT_ERRNO()
#include "skid_file_metadata_read.h"
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */
#define SFMW_ATIME_INDEX 0  // Index of the atime timespec struct
#define SFMW_MTIME_INDEX 1  // Index of the mtime timespec struct


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/
/*
 *  Description:
 *      A wrapper around the call to utimensat().
 *  Args:
 *		pathname: Absolute or relative pathname to modify timestamps for.  If pathname is relative,
 *			pathname will be resolved against the current working directory.
 *		times: The new file timestamps are specified in the array times: times[0] specifies the
 *			new "last access time" (atime); times[1] specifies the new "last modification time"
 *			(mtime).  If the tv_nsec field of one of the timespec structures has the special
 *			value UTIME_NOW, then the corresponding file timestamp is set to the current time.
 *			If the tv_nsec field of one of the timespec structures has the special value
 *			UTIME_OMIT, then the corresponding file timestamp is left unchanged.
 *			If times is NULL, then both timestamps are set to the current time.
 *			In both of these cases, the value of the corresponding tv_sec field is ignored.
 *		follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *			do not follow symbolic links if false.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int call_utnsat(const char *pathname, const struct timespec times[2], bool follow_sym);

/*
 *  Description:
 *      Determine if pathname is absolute.
 *  Args:
 *      pathname: Absolute or relative path.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if absolute, false if not.  On error, returns false and sets errnum.
 */
bool is_abs_path(const char *pathname, int *errnum);

/*
 *	Description:
 *		Set the timespec struct's tv_sec to seconds and the tv_nsec field to nseconds.
 *	Args:
 *		time: A pointer to a timespec struct.
 *		seconds: The epoch seconds to set time to.
 *		nseconds: The nanoseconds to set time to.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int set_timespec(struct timespec *time, time_t seconds, long nseconds);

/*
 *  Description:
 *      Set the timespec struct's tv_nsec field to UTIME_NOW.
 *  Args:
 *		time: A pointer to a timespec struct.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int set_timespec_now(struct timespec *time);

/*
 *  Description:
 *      Set the timespec struct's tv_nsec field to UTIME_OMIT.
 *  Args:
 *		time: A pointer to a timespec struct.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int set_timespec_omit(struct timespec *time);

/*
 *  Description:
 *      Validates the input arguments and updates errnum accordingly.  Will update errnum unless
 *		errnum is the cause of the problem.
 *  Args:
 *      pathname: Must be non-NULL and also can't be empty.
 *      errnum: Must be a non-NULL pointer.  Set to 0 on success.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_sfmw_input(const char *pathname, int *errnum);

/*
 *  Description:
 *      Validates the pathname arguments on behalf of this library.
 *  Args:
 *      pathname: A non-NULL pointer to a non-empty string.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_pathname(const char *pathname);

/*
 *  Description:
 *      Validates the time arguments on behalf of this library.
 *  Args:
 *		time: A pointer to a timespec struct.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_timespec(struct timespec *time);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/
int set_atime(const char *pathname, bool follow_sym, time_t seconds, long nseconds)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // 0 on success, errno on failure
	struct timespec times[2] = {};  // Communicate with utimensat() using atime, mtime

	// INPUT VALIDATION
	result = validate_pathname(pathname);

	// SET IT
	// Prepare atime
	if (ENOERR == result)
	{
		result = set_timespec(&(times[SFMW_ATIME_INDEX]), seconds, nseconds);
	}
	// Prepare mtime
	if (ENOERR == result)
	{
		result = set_timespec_omit(&(times[SFMW_MTIME_INDEX]));
	}
	// Call call_utnsat()
	if (ENOERR == result)
	{
		result = call_utnsat(pathname, times, follow_sym);
	}

	// DONE
	return result;
}


int set_atime_now(const char *pathname, bool follow_sym)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // 0 on success, errno on failure
	struct timespec times[2] = {};  // Communicate with utimensat() using atime, mtime

	// INPUT VALIDATION
	result = validate_pathname(pathname);

	// SET IT
	// Prepare atime
	if (ENOERR == result)
	{
		result = set_timespec_now(&(times[SFMW_ATIME_INDEX]));
	}
	// Prepare mtime
	if (ENOERR == result)
	{
		result = set_timespec_omit(&(times[SFMW_MTIME_INDEX]));
	}
	// Call call_utnsat()
	if (ENOERR == result)
	{
		result = call_utnsat(pathname, times, follow_sym);
	}

	// DONE
	return result;
}


int set_mtime(const char *pathname, bool follow_sym, time_t seconds, long nseconds)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // 0 on success, errno on failure
	struct timespec times[2] = {};  // Communicate with utimensat() using atime, mtime

	// INPUT VALIDATION
	result = validate_pathname(pathname);

	// SET IT
	// Prepare atime
	if (ENOERR == result)
	{
		result = set_timespec_omit(&(times[SFMW_ATIME_INDEX]));
	}
	// Prepare mtime
	if (ENOERR == result)
	{
		result = set_timespec(&(times[SFMW_MTIME_INDEX]), seconds, nseconds);
	}
	// Call call_utnsat()
	if (ENOERR == result)
	{
		result = call_utnsat(pathname, times, follow_sym);
	}

	// DONE
	return result;
}


int set_mtime_now(const char *pathname, bool follow_sym)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // 0 on success, errno on failure
	struct timespec times[2] = {};  // Communicate with utimensat() using atime, mtime

	// INPUT VALIDATION
	result = validate_pathname(pathname);

	// SET IT
	// Prepare atime
	if (ENOERR == result)
	{
		result = set_timespec_omit(&(times[SFMW_ATIME_INDEX]));
	}
	// Prepare mtime
	if (ENOERR == result)
	{
		result = set_timespec_now(&(times[SFMW_MTIME_INDEX]));
	}
	// Call call_utnsat()
	if (ENOERR == result)
	{
		result = call_utnsat(pathname, times, follow_sym);
	}

	// DONE
	return result;
}


int set_times_now(const char *pathname, bool follow_sym)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // 0 on success, errno on failure

	// INPUT VALIDATION
	result = validate_pathname(pathname);

	// Call call_utnsat()
	if (ENOERR == result)
	{
		// If times is NULL, then both timestamps are set to the current time.
		result = call_utnsat(pathname, NULL, follow_sym);
	}

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
int call_utnsat(const char *pathname, const struct timespec times[2], bool follow_sym)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation
	int dirfd = 0;        // dirfd argument for utimensat()
	int flags = 0;        // flags argument for utimensat()

	// INPUT VALIDATION
	retval = validate_pathname(pathname);
	// CALL IT
	// Is pathname absolute?
	if (ENOERR == retval)
	{
		if (false == is_abs_path(pathname, &retval))
		{
			dirfd = AT_FDCWD;  // pathname will be interpreted relative to the calling process's cwd
		}
	}
	// Determine how symbolic links are handled
	if (ENOERR == retval)
	{
		if (false == follow_sym)
		{
			// If pathname specifies a symbolic link, then update the timestamps of the link,
			// rather than the file to which it refers.
			flags |= AT_SYMLINK_NOFOLLOW;
		}
	}
	// Call utimensat()
	if (ENOERR == retval)
	{
		if(utimensat(dirfd, pathname, times, flags))
		{
			retval = errno;
			PRINT_ERROR(The call to utimensat() failed);
			PRINT_ERRNO(retval);
		}
	}

	// DONE
	return retval;
}


bool is_abs_path(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	bool is_abs = false;   // Is pathname absolute? 
	int results = ENOERR;  // The results of validation

	// INPUT VALIDATION
	results = validate_sfmw_input(pathname, errnum);

	// CHECK IT
	if (ENOERR == results)
	{
		if ('/' == *pathname)
		{
			is_abs = true;
		}
	}

	// DONE
	if (errnum)
	{
		*errnum = results;
	}
	return is_abs;
}


int set_timespec(struct timespec *time, time_t seconds, long nseconds)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// INPUT VALIDATION
	retval = validate_timespec(time);

	// SET IT
	if (ENOERR == retval)
	{
		time->tv_sec = seconds;
		time->tv_nsec = nseconds;
	}

	// DONE
	return retval;
}


int set_timespec_now(struct timespec *time)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// INPUT VALIDATION
	retval = validate_timespec(time);

	// SET IT
	if (ENOERR == retval)
	{
		// Set the timespec struct's tv_nsec field to UTIME_NOW
		time->tv_nsec = UTIME_NOW;
	}

	// DONE
	return retval;
}


int set_timespec_omit(struct timespec *time)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// INPUT VALIDATION
	retval = validate_timespec(time);

	// SET IT
	if (ENOERR == retval)
	{
		// Set the timespec struct's tv_nsec field to UTIME_OMIT
		time->tv_nsec = UTIME_OMIT;
	}

	// DONE
	return retval;
}


int validate_sfmw_input(const char *pathname, int *errnum)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// VALIDATE IT
	// pathname
	retval = validate_pathname(pathname);
	// errnum
	if (!errnum)
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received a null errnum pointer);
	}

	// DONE
	if (errnum)
	{
		*errnum = retval;
	}
	return retval;
}


int validate_pathname(const char *pathname)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// VALIDATE IT
	// pathname
	if (!pathname)
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received a null pathname pointer);
	}
	else if (!(*pathname))
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received an empty pathname);
	}

	// DONE
	return retval;
}


int validate_timespec(struct timespec *time)
{
	// LOCAL VARIABLES
	int retval = ENOERR;  // The results of validation

	// VALIDATE IT
	// time
	if (!time)
	{
		retval = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received a null time pointer);
	}

	// DONE
	return retval;
}
