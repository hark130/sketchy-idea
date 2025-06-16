/*
 *  This library defines functionality to create, delete, and empty Linux files.
 */

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // errno
#include <stdbool.h>                        // false
#include <stdio.h>                          // fclose(), fopen(), fread(), fwrite()
#include <string.h>                         // strlen()
#include <unistd.h>                         // unlink()
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_file_metadata_read.h"        // get_size()
#include "skid_file_operations.h"           // bool, empty_file(), false, true
#include "skid_macros.h"                    // ENOERR, SKID_INTERNAL
#include "skid_memory.h"                    // alloc_skid_mem()
#include "skid_validation.h"                // validate_skid_pathname()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Closes *stream, if it's not NULL, using fclose() and sets it to NULL.  This is a
 *      "best effort" function but it will call DEBUG print macros if enabled and fclose() fails.
 *
 *  Args:
 *      stream: A pointer to a file pointer.
 *
 *  Returns:
 *      0 on success, errno on failure.
 */
SKID_INTERNAL int close_stream(FILE **stream);

/*
 *  Description:
 *      Is filename an actual file?  Any invalid input or errno values are treated as a "no".
 *
 *  Args:
 *    filename: Absolute or relative pathname to check.
 *
 *  Returns:
 *    True if filename exists as a file.  False otherwise.
 */
SKID_INTERNAL bool is_file(const char *filename);

/*
 *  Description:
 *      Reads a maximum of buff_size bytes from stream into contents.  This function does not
 *      close stream.  It also does not clear any errors it encountered: end-of-file indicator,
 *      error indicator.
 *
 *  Args:
 *      stream: Open FILE pointer to read from.
 *      contents: [Out] The buffer to read stream into.
 *      buff_size: The maximum number of bytes to read into contents.
 *
 *  Returns:
 *      0, on success.  On failure, an errno value (or -1 for an unspecified error).
 */
SKID_INTERNAL int read_stream(FILE *stream, char *contents, size_t buff_size);

/*
 *  Description:
 *    Validates the pathname arguments on behalf of this library.
 *
 *  Args:
 *    pathname: A non-NULL pointer to a non-empty string.
 *
 *  Returns:
 *    An errno value indicating the results of validation.  ENOERR on successful validation.
 */
SKID_INTERNAL int validate_sfo_pathname(const char *pathname);

/*
 *  Description:
 *      Writes contents to stream and responds to errors.  This function does not close stream.
 *
 *  Args:
 *      contents: The contents to write to stream.
 *      stream: Open FILE pointer to write to.
 *
 *  Returns:
 *      0, on success.  On failure, an errno value (or -1 for an unspecified error).
 */
SKID_INTERNAL int write_stream(const char *contents, FILE *stream);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int create_file(const char *filename, const char *contents, bool overwrite)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution
    FILE *fp = NULL;      // File pointer to filename

    // INPUT VALIDATION
    result = validate_sfo_pathname(filename);
    if (ENOERR == result)
    {
        if (true == is_file(filename) && false == overwrite)
        {
            result = EEXIST;
        }
    }

    // CREATE IT
    // Open it
    if (ENOERR == result)
    {
        fp = fopen(filename, "w");  // Truncate file to zero length or create text file for writing.
        if (!fp)
        {
            result = errno;
            PRINT_ERROR(The call to fopen() failed);
            PRINT_ERRNO(result);
        }
    }
    // Write to it
    if (ENOERR == result)
    {
        if (contents && *contents)
        {
            result = write_stream(contents, fp);
            if (ENOERR != result)
            {
                if (result > 0)
                {
                    PRINT_ERROR(The call to write_stream() failed);
                    PRINT_ERRNO(result);
                }
                else
                {
                    PRINT_ERROR(The call to write_stream() failed with an unspecified error);
                }
            }
        }
    }

    // DONE
    close_stream(&fp);  // Best effort
    return result;
}


int delete_file(const char *filename)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution

    // INPUT VALIDATION
    result = validate_sfo_pathname(filename);

    // DELETE IT
    if (ENOERR == result)
    {
        if (unlink(filename))
        {
            result = errno;
            PRINT_ERROR(The call to unlink() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


int empty_file(const char *filename)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution

    // INPUT VALIDATION
    // Let create_file() handle input validation

    // CREATE IT
    result = create_file(filename, NULL, true);

    // DONE
    return result;
}


char *read_file(const char *filename, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;    // Results of execution
    char *contents = NULL;  // File contents
    off_t file_size = 0;    // File size in bytes
    FILE *fp = NULL;        // File pointer to filename

    // INPUT VALIDATION
    if (false == is_file(filename))
    {
        result = ENOENT;  // No such file
    }
    else if (!errnum)
    {
        result = EINVAL;  // Bad input
    }

    // READ IT
    // Size it
    if (ENOERR == result)
    {
        file_size = get_size(filename, &result);
        if (result)
        {
            PRINT_ERROR(The call to get_size() failed);
            PRINT_ERRNO(result);
        }
    }
    // Allocate it
    if (ENOERR == result)
    {
        contents = alloc_skid_mem(file_size + 1, 1, &result);
        if (result)
        {
            PRINT_ERROR(The call to alloc_skid_mem() failed);
            PRINT_ERRNO(result);
        }
    }
    // Open it
    if (ENOERR == result)
    {
        fp = fopen(filename, "r");
        if (!fp)
        {
            result = errno;
            PRINT_ERROR(The call to fopen() failed);
            PRINT_ERRNO(result);
        }
    }
    // Read it
    if (ENOERR == result)
    {
        result = read_stream(fp, contents, file_size);
        if (result)
        {
            PRINT_ERROR(The call to read_stream() failed);
            PRINT_ERRNO(result);
        }
    }

    // CLEANUP
    close_stream(&fp);  // Best effort
    if (result && contents)
    {
        free_skid_mem((void **)&contents);  // Free the memory since there was an error
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return contents;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL int close_stream(FILE **stream)
{
    // LOCAL VARIABLES
    int errnum = ENOERR;  // Store errno values here

    // INPUT VALIDATION
    if (stream && *stream)
    {
        if (fclose(*stream))
        {
            errnum = errno;
            PRINT_ERROR(The call to fclose() failed);
            PRINT_ERRNO(errnum);
        }
        *stream = NULL;  // Succeed or fail, we tried...
    }

    // DONE
    return errnum;
}


SKID_INTERNAL bool is_file(const char *filename)
{
    // LOCAL VARIABLES
    bool is_a_file = false;  // Results of execution
    int errnum = ENOERR;     // Store errno values here

    // INPUT VALIDATION
    is_a_file = is_regular_file(filename, &errnum);

    // DONE
    return is_a_file;
}


SKID_INTERNAL int read_stream(FILE *stream, char *contents, size_t buff_size)
{
    // LOCAL VARIABLES
    int result = ENOERR;    // The results of validation
#ifdef SKID_DEBUG
    size_t bytes_read = 0;  // Return value from fread()
#endif  /* SKID_DEBUG */

    // INPUT VALIDATION
    if (!stream || !contents || buff_size <= 0)
    {
        result = EINVAL;  // Bad input
    }

    // READ IT
    if (ENOERR == result)
    {
#ifdef SKID_DEBUG
        bytes_read = fread(contents, buff_size, 1, stream);
#else
        fread(contents, buff_size, 1, stream);
#endif  /* SKID_DEBUG */
        if (feof(stream))
        {
            // Great news.  Continue...
            FPRINTF_ERR("%s - Read to the EOF in read_stream()\n", DEBUG_INFO_STR);
        }
        if (ferror(stream))
        {
            PRINT_ERROR(The call to fread() failed with an unspecified error);
            result = EIO;  // Seems accurate
        }
        else
        {
            FPRINTF_ERR("%s - Read %zu bytes in read_stream()\n", DEBUG_INFO_STR, bytes_read);
        }
    }

    // DONE
    return result;
}


SKID_INTERNAL int validate_sfo_pathname(const char *pathname)
{
    return validate_skid_pathname(pathname, false);  // Refactored for backwards compatibility
}


SKID_INTERNAL int write_stream(const char *contents, FILE *stream)
{
    // LOCAL VARIABLES
    int result = ENOERR;         // The results of validation
    size_t nmemb = 0;            // Number of characters in contents
    size_t num_items_wrote = 0;  // The number of items written by fwrite()

    // INPUT VALIDATION
    if (!contents || !(*contents) || !stream)
    {
        result = EINVAL;  // NULL pointer or empty buffer
    }
    else
    {
        nmemb = strlen(contents);
    }

    // WRITE IT
    if (ENOERR == result)
    {
        num_items_wrote = fwrite(contents, sizeof(*contents), nmemb, stream);
        if (0 == num_items_wrote)
        {
            PRINT_ERROR(The call to fwrite() failed);
            PRINT_ERRNO(result);
            result = -1;  // Failure!
        }
        else if (num_items_wrote < nmemb)
        {
            PRINT_WARNG(fwrite() performed a short write);
            PRINT_ERRNO(result);
            result = -1;  // Failure?
        }
        else if (num_items_wrote != nmemb)
        {
            PRINT_WARNG(fwrite() performed an OVER-write?!);
            PRINT_ERRNO(result);
            result = -1;  // Failure?!
        }
    }

    // DONE
    return result;
}
