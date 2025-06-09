/*
 *    This library defines functionality to modify Linux file metadata.
 */

#define _POSIX_C_SOURCE 200809L             // Expose utimensat()
// #define SKID_DEBUG                          // Enable DEBUG logging

#include <fcntl.h>                          // AT_FDCWD
#include "skid_debug.h"                     // PRINT_ERRNO()
#include "skid_file_metadata_read.h"        // get_file_perms()
#include "skid_file_metadata_write.h"       // set_mode()
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */
#define SFMW_ATIME_INDEX 0  // Index of the atime timespec struct
#define SFMW_MTIME_INDEX 1  // Index of the mtime timespec struct
#define SFMW_IGNORE_ID -1   // When using the chown-family funcs, if an ID is -1 it is not changed.


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/
/*
 *  Description:
 *      Calls one of the chown-family functions based on caller arguments.  Defaults to chown().
 *        If pathname is a symbolic link and follow_sym is false, uses lchown() instead.
 *  Args:
 *      pathname: Absolute or relative pathname to update with a chown-family function.
 *      new_owner: The UID of the new owner for pathname.  Pass -1 to ignore this ID.
 *      new_group: The GID of the new group for pathname.  Pass -1 to ignore this ID.
 *      follow_sym: If true, calls chown().  If false, uses lchown() for symlinks.
 *  Returns:
 *      0 on success.  Errno value on failure.
 */
int call_a_chown(const char *pathname, uid_t new_owner, gid_t new_group, bool follow_sym);

/*
 *  Description:
 *      Calls chown(pathname, new_owner, new_group).
 *  Args:
 *      pathname: Absolute or relative pathname to update with chown().
 *      new_owner: The UID of the new owner for pathname.  Pass -1 to ignore this ID.
 *      new_group: The GID of the new group for pathname.  Pass -1 to ignore this ID.
 *  Returns:
 *      0 on success.  Errno value on failure.
 */
int call_chown(const char *pathname, uid_t new_owner, gid_t new_group);

/*
 *  Description:
 *      Calls lchown(pathname, new_owner, new_group).
 *  Args:
 *      pathname: Absolute or relative pathname to update with lchown().
 *      new_owner: The UID of the new owner for pathname.  Pass -1 to ignore this ID.
 *      new_group: The GID of the new group for pathname.  Pass -1 to ignore this ID.
 *  Returns:
 *      0 on success.  Errno value on failure.
 */
int call_lchown(const char *pathname, uid_t new_owner, gid_t new_group);

/*
 *  Description:
 *      A wrapper around the call to utimensat().
 *  Args:
 *      pathname: Absolute or relative pathname to modify timestamps for.  If pathname is relative,
 *          pathname will be resolved against the current working directory.
 *      times: The new file timestamps are specified in the array times: times[0] specifies the
 *          new "last access time" (atime); times[1] specifies the new "last modification time"
 *          (mtime).  If the tv_nsec field of one of the timespec structures has the special
 *          value UTIME_NOW, then the corresponding file timestamp is set to the current time.
 *          If the tv_nsec field of one of the timespec structures has the special value
 *          UTIME_OMIT, then the corresponding file timestamp is left unchanged.
 *          If times is NULL, then both timestamps are set to the current time.
 *          In both of these cases, the value of the corresponding tv_sec field is ignored.
 *      follow_sym: Controls how symbolic links are handled: follow symbolic links if true,
 *          do not follow symbolic links if false.
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
 *  Description:
 *      Set the timespec struct's tv_sec to seconds and the tv_nsec field to nseconds.
 *  Args:
 *      time: A pointer to a timespec struct.
 *      seconds: The epoch seconds to set time to.
 *      nseconds: The nanoseconds to set time to.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int set_timespec(struct timespec *time, time_t seconds, long nseconds);

/*
 *  Description:
 *      Set the timespec struct's tv_nsec field to UTIME_NOW.
 *  Args:
 *      time: A pointer to a timespec struct.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int set_timespec_now(struct timespec *time);

/*
 *  Description:
 *      Set the timespec struct's tv_nsec field to UTIME_OMIT.
 *  Args:
 *      time: A pointer to a timespec struct.
 *  Returns:
 *      0 on success.  An errno value on failure.
 */
int set_timespec_omit(struct timespec *time);

/*
 *  Description:
 *      Validates the input arguments and updates errnum accordingly.  Will update errnum unless
 *      errnum is the cause of the problem.
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
 *      time: A pointer to a timespec struct.
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_timespec(struct timespec *time);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int add_mode(const char *pathname, mode_t more_mode)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // 0 on success, errno on failure
    mode_t old_mode = 0;  // Read this with get_file_perms()
    mode_t new_mode = 0;  // Set this with set_mode()

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // ADD IT UP
    // 1. Get current mode
    if (ENOERR == result)
    {
        old_mode = get_file_perms(pathname, &result);
    }
    // 2. Add more_mode
    if (ENOERR == result)
    {
        new_mode = old_mode | more_mode;
    }
    // 3. Call set_mode()
    if (ENOERR == result)
    {
        result = set_mode(pathname, new_mode);
    }

    // DONE
    return result;
}


int remove_mode(const char *pathname, mode_t less_mode)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // 0 on success, errno on failure
    mode_t old_mode = 0;  // Read this with get_file_perms()
    mode_t new_mode = 0;  // Set this with set_mode()

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // ADD IT UP
    // 1. Get current mode
    if (ENOERR == result)
    {
        old_mode = get_file_perms(pathname, &result);
    }
    // 2. Remove less_mode
    if (ENOERR == result)
    {
        new_mode = old_mode & (~less_mode);
    }
    // 3. Call set_mode()
    if (ENOERR == result)
    {
        result = set_mode(pathname, new_mode);
    }

    // DONE
    return result;
}


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


int set_group_id(const char *pathname, gid_t new_group, bool follow_sym)
{
    // LOCAL VARIABLES
    int result = ENOERR;         // 0 on success, errno on failure
    uid_t uid = SFMW_IGNORE_ID;  // Ignore the owner

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // SET IT
    if (ENOERR == result)
    {
        result = call_a_chown(pathname, uid, new_group, follow_sym);
    }

    // DONE
    return result;
}


int set_mode(const char *pathname, mode_t new_mode)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // 0 on success, errno on failure

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // SET IT
    if (ENOERR == result)
    {
        if (chmod(pathname, new_mode))
        {
            result = errno;
            PRINT_ERROR(The call to chmod() failed);
            PRINT_ERRNO(result);
        }
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


int set_owner_id(const char *pathname, uid_t new_owner, bool follow_sym)
{
    // LOCAL VARIABLES
    int result = ENOERR;         // 0 on success, errno on failure
    gid_t gid = SFMW_IGNORE_ID;  // Ignore the group

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // SET IT
    if (ENOERR == result)
    {
        result = call_a_chown(pathname, new_owner, gid, follow_sym);
    }

    // DONE
    return result;
}


int set_ownership(const char *pathname, uid_t new_owner, gid_t new_group, bool follow_sym)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // 0 on success, errno on failure

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // SET IT
    if (ENOERR == result)
    {
        result = call_a_chown(pathname, new_owner, new_group, follow_sym);
    }

    // DONE
    return result;
}


int set_times(const char *pathname, bool follow_sym, time_t seconds, long nseconds)
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
int call_a_chown(const char *pathname, uid_t new_owner, gid_t new_group, bool follow_sym)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // The results of execution
    bool is_sym = false;  // Is pathname a symbolic link?

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // CALL IT
    // Is pathname a symbolic link?
    if (ENOERR == result)
    {
        is_sym = is_sym_link(pathname, &result);
    }
    // Call it
    if (ENOERR == result)
    {
        if (true == is_sym && false == follow_sym)
        {
            result = call_lchown(pathname, new_owner, new_group);
        }
        else
        {
            result = call_chown(pathname, new_owner, new_group);
        }
    }

    // DONE
    return result;
}


int call_chown(const char *pathname, uid_t new_owner, gid_t new_group)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // The results of execution

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // CALL IT
    if (ENOERR == result)
    {
        if (chown(pathname, new_owner, new_group))
        {
            result = errno;
            PRINT_ERROR(The call to chown() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


int call_lchown(const char *pathname, uid_t new_owner, gid_t new_group)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // The results of execution

    // INPUT VALIDATION
    result = validate_pathname(pathname);

    // CALL IT
    if (ENOERR == result)
    {
        if (lchown(pathname, new_owner, new_group))
        {
            result = errno;
            PRINT_ERROR(The call to lchown() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


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
