#ifndef __SKID_FILE_OPERATIONS__
#define __SKID_FILE_OPERATIONS__

#include <stdbool.h>    // bool, false, true

/*
 *  Description:
 *      Write entry to the end of filename.
 *
 *  Args:
 *      filename: Absolute or relative filename to write to.
 *      entry: The entry to write to the end of filename.
 *      create: If true, will create filename if it doesn't already exist.  If create is false
 *          and filename does not exist, returns ENOENT.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.  Uses EISDIR if filename exists but
 *      is not a file.
 */
int append_to_file(const char *filename, const char *entry, bool create);

/*
 *  Description:
 *      Created a filename with provided contents.
 *
 *  Args:
 *      filename: Absolute or relative filename to create.
 *      contents: [Optional] The contents of filename.  Ignored if NULL or empty.
 *      overwrite: If true, will overwrite the contents of a pre-existing filename with contents.
 *          If overwrite is true but contents is NULL/empty, then filename will be emptied.
 *          If overwrite is false and filename exists, will return EEXIST.
 *
 *  Returns:
 *      0, on success.  On failure, an errno value (or -1 for an unspecified error).
 */
int create_file(const char *filename, const char *contents, bool overwrite);

/*
 *  Description:
 *      Deletes filename by calling unlink().
 *
 *  Notes:
 *      If that name was the last link to a file and no processes have the file open,
 *          the file is deleted and the space it was using is made available for reuse.
 *      If the name was the last link to a file but any processes still have the file open,
 *          the file will remain in existence until the last file descriptor referring to it is
 *          closed.
 *      If the name referred to a symbolic link, the link is removed.
 *      If the name referred to a socket, FIFO, or device, the name for it is removed but
 *          processes which have the object open may continue to use it.
 *
 *  Args:
 *      filename: Absolute or relative filename to deleted.
 *
 *  Returns:
 *      0, on success.  On failure, an errno value.
 */
int delete_file(const char *filename);

/*
 *  Description:
 *      Removes all contents from filename.  Will create the file if it doesn't exist.
 *
 *  Args:
 *      filename: Absolute or relative filename to emptied.
 *
 *  Returns:
 *      0, on success.  On failure, an errno value (or -1 for an unspecified error).
 */
int empty_file(const char *filename);

/*
 *  Description:
 *      Read the contents of filename into a heap-allocated buffer.  It is the caller's
 *      responsibility to free the buffer with free_skid_mem().
 *
 *  Args:
 *      filename: Absolute or relative filename to read.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Pointer, on success.  NULL on error (check errnum for details).
 */
char *read_file(const char *filename, int *errnum);

#endif  /* __SKID_FILE_OPERATIONS__ */
