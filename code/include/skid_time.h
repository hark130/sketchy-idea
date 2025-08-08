#ifndef __SKID_TIME__
#define __SKID_TIME__


#define SKID_BAD_TIME_T ((time_t)-1)

#include <time.h>               // time_t


/*
 *  Description:
 *      Convert the Unix Epoch into local time inside a statically allocated tm struct.
 *      This statically allocated struct might be overwritten by subsequent calls to any of the date
 *      and time functions.  Do *not* call free() on the return value.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Pointer to a statically allocated struct on success.  NULL on error (check errnum for details).
 *      ETIMEDOUT is used to communicate indiscriminate errors.
 */
struct tm *get_localtime(int *errnum);

/*
 *  Description:
 *      Fetch the Unix Epoch time.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC), on success.
 *      SKID_BAD_TIME_T on error (check errnum for details).
 */
time_t get_unix_time(int *errnum);

/*
 *  Description:
 *      Translate the current local time into a YYYYMMDD-HHMMSS string format.
 *      The caller is responsible for using free_skid_mem() to free the return value.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated, nul-terminated, datetime-stamp formatted string, on success.
 *      NULL on error (check errnum for details).
 */
char *build_timestamp(int *errnum);

/*
 *  Description:
 *      Preface a message with a formatted timestamp.  Use the delims as bookends for the
 *      timestamp.  Uses build_timestap() to get the datetime stamp.
 *      The caller is responsible for using free_skid_mem() to free the return value.
 *
 *  Usage:
 *      timestamp_a_msg("This is my message\n", "[]", &errnum);  // Should return...
 *      // "[20250721-124356] This is my message\n"
 *
 *  Args:
 *      msg: The message to add a delimited timestamp to.
 *      delims: The delimiters to add to the message.  Nul characters (e.g., '\0', 0x0) will
 *          be ignored/skipped.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated, nul-terminated, datetime-stamp formatted msg, on success.
 *      NULL on error (check errnum for details).
 */
char *timestamp_a_msg(const char *msg, const char delims[2], int *errnum);


#endif  /* __SKID_TIME__ */
