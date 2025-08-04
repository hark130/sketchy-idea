#ifndef __SKID_VALIDATION__
#define __SKID_VALIDATION__

#include <stdbool.h>                        // bool, false, true
#include "skid_macros.h"                    // ENOERR, SKID_BAD_FD

/*
 *  Description:
 *      Validates the errnum arguments on behalf of SKID.
 *
 *  Args:
 *      err: A non-NULL pointer to an integer.
 *
 *  Returns:
 *      An errno value indicating the results of validation.  ENOERR on successful validation.
 */
int validate_skid_err(int *err);

/*
 *  Description:
 *      Validate file descriptors on behalf of SKID.
 *
 *  Args:
 *      fd: File descriptor to validate.
 *
 *  Returns:
 *      ENOERR on success, EBADF on failed validation.
 */
int validate_skid_fd(int fd);

/*
 *  Description:
 *      Validates the pathname argument on behalf of SKID.
 *
 *  Args:
 *      pathname: A non-NULL pointer to a non-empty string.
 *      must_exist: If true, this function will use lstat() to access pathname.
 *
 *  Returns:
 *      ENOERR on successful validation.  An errno value indicating the results of validation:
 *          - EINVAL: pathname is NULL or empty.
 *          - ENOENT: must_exist is true but pathname was not found.
 */
int validate_skid_pathname(const char *pathname, bool must_exist);

/*
 *  Description:
 *      Validate shared object names (e.g., shared memory, named semaphores) on behalf of SKID.
 *
 *  Args:
 *      shared_name: Shared object name to validate.
 *      must_port: If true, shared_name must begin with a '/' for the sake of portability.
 *          If you feel compelled to use false here, consider calling
 *          validate_skid_string(name, false) for more gentle validation
 *          (or use validate_skid_pathname(name, true) if you're using an actual filename?!).
 *
 *  Returns:
 *      ENOERR on success, errno on failed validation.
 */
int validate_skid_shared_name(const char *shared_name, bool must_port);

/*
 *  Description:
 *      Validate socket file descriptors on behalf of SKID.
 *
 *  Args:
 *      sockfd: Socket file descriptor to validate.
 *
 *  Returns:
 *      ENOERR on success, errno on failed validation.
 */
int validate_skid_sockfd(int sockfd);

/*
 *  Description:
 *      Validate C strings on behalf of SKID.
 *
 *  Args:
 *      string: The C string to validate.
 *      can_be_empty: If true, string may be empty.  Otherwise, the string must have a length.
 *
 *  Returns:
 *      ENOERR on success, errno on failed validation.
 */
int validate_skid_string(const char *string, bool can_be_empty);

#endif  /* __SKID_VALIDATION__ */
