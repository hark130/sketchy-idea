#ifndef __SKID_PIPES__
#define __SKID_PIPES__

/*
 *  Description:
 *      Make a FIFO special file with name pathname and permissions mode.
 *
 *  Args:
 *		pathname: Absolute or relative pathname for the new FIFO special file.
 *		mode: Specifies pathname's permissions.  See: skid_macros.h for mode macros.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.  Uses EEXIST if pathname exists.
 */
int make_named_pipe(const char *pathname, mode_t mode, int *errnum);

#endif  /* __SKID_PIPES__ */
