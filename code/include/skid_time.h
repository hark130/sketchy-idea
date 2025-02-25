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


#endif  /* __SKID_TIME__ */
