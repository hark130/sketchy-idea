/*
 *  This library defines functionality to handle unnamed and named semaphores.
 */

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // errno
#include <fcntl.h>                          // O_CREAT
#include "skid_debug.h"                     // MODULE_*LOAD(), PRINT_ERR*()
#include "skid_macros.h"                    // ENOERR, SKID_BAD_SEM_PTR, SKID_INTERNAL
#include "skid_semaphores.h"                // named_sem_t, public functions
#include "skid_validation.h"                // validate_skid_*()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Standardize the way sem_open() is called and responds to errors.
 *
 *  Args:
 *      name: The name of the POSIX semaphore to open.
 *      oflag: Controls the operation of the call to sem_open().
 *      mode: [Optional] Specifies the permissions to be placed on the new semaphore.  If O_CREAT is
 *          specified, and a semaphore with the given name already exists, then mode is ignored.
 *      value: [Optional] Specifies the initial value for the new semaphore.  If O_CREAT is
 *          specified, and a semaphore with the given name already exists, then value is ignored.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Address of the new semaphore on success.  SKID_BAD_SEM_PTR on failure and errnum is
 *      set with an errno value.
 */
SKID_INTERNAL sem_t *call_sem_open(const char *name, int oflag, mode_t mode,
                                   unsigned int value, int *errnum);

/*
 *  Description:
 *      Validate a semaphore pointer on behalf of this library.  Normally, I would define this
 *      in skid_validation but this seems too niche to be public.
 *
 *  Args:
 *      sem_ptr: Semaphore pointer to validate.
 *
 *  Returns:
 *      ENOERR on success, EINVAL on failed validation.
 */
SKID_INTERNAL int validate_sem_ptr(sem_t *sem_ptr);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int close_named_sem(named_sem_ptr *old_named_sem)
{
    // LOCAL VARIABLES
    int results = ENOERR;                      // Store errno values
    named_sem_ptr sem_ptr = SKID_BAD_SEM_PTR;  // The named semaphore pointer to close

    // INPUT VALIDATION
    if (NULL == old_named_sem)
    {
        results = EINVAL;  // NULL pointer
    }
    else
    {
        sem_ptr = *old_named_sem;
    }

    // CLOSE IT
    if (ENOERR == results)
    {
        if (0 != sem_close(sem_ptr))
        {
            results = errno;
            PRINT_ERROR(The call to sem_close() failed);
            PRINT_ERRNO(results);
        }
        else
        {
            *old_named_sem = SKID_BAD_SEM_PTR;
        }
    }

    // DONE
    return results;
}


named_sem_ptr create_named_sem(const char *name, int flags, mode_t mode, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;                      // Store errno values
    int new_flags = flags | O_CREAT;           // This call should create name
    named_sem_ptr sem_ptr = SKID_BAD_SEM_PTR;  // Pointer to the new named semaphore
    int value = 1;                             // Initial value of the semaphore

    // INPUT VALIDATION
    // The name arg is validated by call_sem_open()
    results = validate_skid_err(errnum);

    // CREATE IT
    if (ENOERR == results)
    {
        sem_ptr = (named_sem_ptr)call_sem_open(name, new_flags, mode, value, &results);
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return sem_ptr;
}


named_sem_ptr open_named_sem(const char *name, int flags, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;                      // Store errno values
    mode_t mode = 0;                           // Dummy sem_open(mode) value
    int value = 0;                             // Dummy sem_open(value) value
    named_sem_ptr sem_ptr = SKID_BAD_SEM_PTR;  // Pointer to the named semaphore

    // INPUT VALIDATION
    // The name arg is validated by call_sem_open()
    results = validate_skid_err(errnum);
    if (ENOERR == results)
    {
        if (O_CREAT == (O_CREAT & flags) && 0 != O_CREAT)
        {
            results = EINVAL;  // Call create_named_sem() instead
            PRINT_ERROR(Detected the CREATE flag in flags);
        }
    }

    // OPEN IT
    if (ENOERR == results)
    {
        sem_ptr = (named_sem_ptr)call_sem_open(name, flags, mode, value, &results);
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return sem_ptr;
}


int release_sem(sem_t *semaphore)
{
    // LOCAL VARIABLES
    int results = validate_sem_ptr(semaphore);  // Store errno values

    // INPUT VALIDATION
    if (ENOERR == results)
    {
        if (0 != sem_post(semaphore))
        {
            results = errno;
            PRINT_ERROR(The call to sem_post() failed);
            PRINT_ERRNO(results);
        }
    }

    // DONE
    return results;
}


int remove_named_sem(const char *name)
{
    // LOCAL VARIABLES
    int results = ENOERR;                      // Store errno values

    // INPUT VALIDATION
    results = validate_skid_shared_name(name, true);  // Must be portable

    // REMOVE IT
    if (ENOERR == results)
    {
        if (0 != sem_unlink(name))
        {
            results = errno;
            PRINT_ERROR(The call to sem_unlink() failed);
            PRINT_ERRNO(results);
        }
    }

    // DONE
    return results;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL sem_t *call_sem_open(const char *name, int oflag, mode_t mode,
                                   unsigned int value, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;               // Store errno values
    sem_t *sem_ptr = SKID_BAD_SEM_PTR;  // Pointer to the new semaphore

    // INPUT VALIDATION
    results = validate_skid_shared_name(name, true);  // Must be portable
    if (ENOERR == results)
    {
        results = validate_skid_err(errnum);
    }

    // CALL IT
    if (ENOERR == results)
    {
        errno = ENOERR;  // Reset errno
        sem_ptr = sem_open(name, oflag, mode, value);
        if (SEM_FAILED == sem_ptr || NULL == sem_ptr)
        {
            results = errno;
            if (NULL == sem_ptr)
            {
                PRINT_ERROR(The call to sem_open() failed but returned NULL);
            }
            if (ENOERR == results)
            {
                PRINT_ERROR(Handling an unspecified error);
                results = ENOTNAM;  // <shrug>
            }
            PRINT_ERROR(The call to sem_open() failed);
            PRINT_ERRNO(results);
            sem_ptr = SKID_BAD_SEM_PTR;
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return sem_ptr;
}


SKID_INTERNAL int validate_sem_ptr(sem_t *sem_ptr)
{
    // LOCAL VARIABLES
    int results = EINVAL;  // Store errno values

    // INPUT VALIDATION
    if (SKID_BAD_SEM_PTR == sem_ptr)
    {
        PRINT_ERROR(Bad semaphore pointer detected);
    }
    else if (NULL == sem_ptr)
    {
        PRINT_ERROR(NULL semaphore pointer detected);
    }
    else
    {
        results = ENOERR;  // It's good
    }

    // DONE
    return results;
}
