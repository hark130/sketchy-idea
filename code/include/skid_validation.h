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
 *      ENOERR on success, errno on failed validation.
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
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_skid_pathname(const char *pathname, bool must_exist);

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
