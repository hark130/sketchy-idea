/*
 *  This library defines time-related functionality.
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // errno
#include <stdio.h>                          // sprintf()
#include <string.h>                         // strlen()
#include "skid_debug.h"                     // PRINT_ERRNO, PRINT_ERROR
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // alloc_skid_mem(), free_skid_mem()
#include "skid_time.h"                      // SKID_BAD_TIME_T, time.h
#include "skid_validation.h"                // validate_skid_err()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


struct tm *get_localtime(int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Results of execution
    time_t num_secs = 0;      // Number of seconds since the Unix Epoch
    struct tm *ltime = NULL;  // Local time struct

    // INPUT VALIDATION
    result = validate_skid_err(errnum);

    // GET IT
    // Get Unix Epoch
    if (ENOERR == result)
    {
        num_secs = get_unix_time(&result);
        if (SKID_BAD_TIME_T == num_secs)
        {
            PRINT_ERROR(The call to get_unix_time() failed);
            PRINT_ERRNO(result);
        }
    }
    // Get Localtime
    if (ENOERR == result)
    {
        ltime = localtime(&num_secs);
        if (NULL == ltime)
        {
            result = errno;
            PRINT_ERROR(The call to localtime() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return ltime;
}


time_t get_unix_time(int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution
    time_t num_secs = 0;  // Number of seconds since the Unix Epoch

    // INPUT VALIDATION
    result = validate_skid_err(errnum);

    // GET IT
    if (ENOERR == result)
    {
        num_secs = time(NULL);
        if (SKID_BAD_TIME_T == num_secs)
        {
            PRINT_ERROR(The call to time() failed);
            result = errno;
            if (ENOERR == result)
            {
                result = ETIMEDOUT;  // It failed but there's no errno?  Use ETIMEDOUT.
            }
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return num_secs;
}


char *build_timestamp(int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;                        // Results of execution
    char *timestamp = NULL;                     // Heap allocated timestamp buffer
    size_t ts_len = strlen("YYYYMMDD-HHMMSS");  // Length of the datetime stamp
    struct tm *dt_struct = NULL;

    // INPUT VALIDATION
    result = validate_skid_err(errnum);

    // BUILD IT
    // Allocate memory
    if (ENOERR == result)
    {
        timestamp = (char *)alloc_skid_mem(ts_len + 1, sizeof(char), &result);
    }
    // Get the local time
    if (ENOERR == result)
    {
        dt_struct = get_localtime(&result);
    }
    // Format the local time
    if (ENOERR == result)
    {
        sprintf(timestamp, "%04d%02d%02d-%02d%02d%02d", dt_struct->tm_year+1900,
                dt_struct->tm_mon, dt_struct->tm_mday, dt_struct->tm_hour,
                dt_struct->tm_min, dt_struct->tm_sec);
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return timestamp;
}
