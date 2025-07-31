#ifndef __SKID_SEMAPHORES__
#define __SKID_SEMAPHORES__


#include <semaphore.h>                      // sem_t
#include "skid_macros.h"                    // ENOERR


/*
 * This data type is an attempt to clearly communicate the difference between semaphores and
 * named semaphores.  Functions defined here that can safely operate on both will take the sem_t
 * data type.  Functions that only safely operate on named semaphores will take the named_sem_t
 * data type instead.
 */
typedef sem_t named_sem_t, *named_sem_ptr;  // Named semaphore (see: sem_overview(7))

/*
 *  Description:
 *      Close a named semaphore by calling sem_close() and set the
 *      named_sem_ptr to SKID_BAD_SEM_PTR.
 *
 *  Args:
 *      old_named_sem: [In/Out] The pointer to the named semaphore pointer to close.
 *
 *  Returns:
 *      ENOERR on success, errno value on error.
 */
int close_named_sem(named_sem_ptr *old_named_sem);

/*
 *  Description:
 *      Create a new named semaphore that is locked.  The named semaphore will be initialized
 *      with a value of 1.  The caller is responsible for calling close_named_sem() on the return
 *      value and then remove_named_sem() on name.
 *
 *  Args:
 *      name: The name of the named semaphore.  Must begin with '/' for portability.
 *      flags: Controls the operation of the call to sem_open().  This value will be ORed with
 *          O_CREAT.  Definitions of the flag values can be obtained by including <fcntl.h>.
 *      mode: Specifies the permissions to be placed on the new semaphore.  Utilize the skid_macro.h
 *          SKID_MODE_* macros or see open(2).
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      The address of the new semaphore, on success.  SKID_BAD_SEM_PTR on error and sets
 *      errnum accordingly.  Uses ENOTNAM for unspecified errors.
 */
named_sem_ptr create_named_sem(const char *name, int flags, mode_t mode, int *errnum);

/*
 *  Description:
 *      Releases the semaphore by calling sem_post().
 *
 *  Args:
 *      semaphore: Pointer to the semaphore to release.
 *
 *  Returns:
 *      ENOERR on success, errno on error.
 */
int release_sem(sem_t *semaphore);

/*
 *  Description:
 *      Remove a named semaphore by calling sem_unlink().
 *
 *  Args:
 *      name: The name of the named semaphore.  Must begin with '/' for portability.
 *
 *  Returns:
 *      ENOERR on success, errno on error.
 */
int remove_named_sem(const char *name);

#endif  /* __SKID_SEMAPHORES__ */
