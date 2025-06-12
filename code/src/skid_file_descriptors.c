/*
 *    This library defines functionality to manage Linux file descriptors.
 */

#include <errno.h>                      // EINVAL
#include <fcntl.h>                      // open()
#include <stddef.h>                     // size_t
#include <string.h>                     // strlen()
#include <unistd.h>                     // close()
#include "skid_debug.h"                 // PRINT_ERROR()
#include "skid_file_descriptors.h"      // close_fd()
#include "skid_macros.h"                // ENOERR, SKID_BAD_FD, SKID_INTERNAL
#include "skid_memory.h"                // alloc_skid_mem(), free_skid_mem()
#include "skid_validation.h"            // validate_skid_fd(), validate_skid_string()

#define SKID_FD_BUFF_SIZE 1024  // Starting buffer size to read into


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Determine if the bytes_read can fit into the output buffer based on its size and current
 *      length.
 *
 *  Args:
 *      bytes_read: The number of bytes to append to the output buffer.
 *      output_len: The number of bytes currently in the buffer.
 *      output_size: Total size of the output buffer.
 *
 *  Returns:
 *      True if there's room.  False if there isn't (time to reallocate), on for invalid args.
 */
SKID_INTERNAL bool check_for_space(size_t bytes_read, size_t output_len, size_t output_size);

/*
 *  Description:
 *      Check for an existing buffer pointer.  If one does not exist, make the first allocation.
 *
 *  Args:
 *      output_buf: [Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *          a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *          output_size will be updated.
 *      output_size: [Out] Pointer to the size of output_buf.
 *
 *  Returns:
 *      0 on success, errno on failure.
 */
SKID_INTERNAL int check_for_pre_alloc(char **output_buf, size_t *output_size);

/*
 *  Description:
 *      Read the contents of the file descriptor into a heap-allocated buffer.  If the buffer ever
 *      fills then this function will (effectively) reallocate more space.  It will read until
 *      no other data can be read or an error occurred.  It is the caller's responsibility to
 *      free the buffer with free_skid_mem().
 *
 *  Args:
 *      fd: File descriptor to read from.
 *      output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *          a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *          output_size will be updated.
 *      output_size: [In/Out] Pointer to the size of output_buf.
 *
 *  Returns:
 *      0 on success, errno on failure.
 */
SKID_INTERNAL int read_fd_dynamic(int fd, char **output_buf, size_t *output_size);

/*
 *  Description:
 *      Reallocate a buffer: allocate a new buffer double the *output_size, copy the contents of
 *      *output_buf into the new buffer, free the old buffer, update the Out arguments.
 *      It is the caller's responsibility to free the buffer with free_skid_mem().
 *
 *  Args:
 *      output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *          a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *          output_size will be updated.
 *      output_size: [In/Out] Pointer to the size of output_buf.
 *
 *  Returns:
 *      0 on success, errno on failure.  EOVERFLOW is used to indicate the output_size can not
 *      be doubled without overflowing the size_t data type.
 */
SKID_INTERNAL int realloc_fd_dynamic(char **output_buf, size_t *output_size);

/*
 *  Description:
 *      Validate common In/Out args on behalf of the library.
 *
 *  Args:
 *      output_buf: [In/Out] Pointer to the working heap-allocated buffer.  If this pointer holds
 *          a NULL pointer, heap memory will be allocated, the pointer will be stored here, and
 *          output_size will be updated.
 *      output_size: [In/Out] Pointer to the size of output_buf.
 *
 *  Returns:
 *      0 on success, errno on failed validation.
 */
SKID_INTERNAL int validate_sfd_args(char **output_buf, size_t *output_size);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int close_fd(int *fdp, bool quiet)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values

    // INPUT VALIDATION
    if (NULL == fdp)
    {
        result = EINVAL;  // NULL pointer
    }
    else
    {
        result = validate_skid_fd(*fdp);
    }

    // CLOSE IT
    if (ENOERR == result)
    {
        if (close(*fdp))
        {
            // close() failed
            result = errno;
            if (false == quiet)
            {
                PRINT_ERROR(The call to close() failed);
                PRINT_ERRNO(result);
            }
        }
        else
        {
            *fdp = SKID_BAD_FD;
        }
    }

    // DONE
    return result;
}


int call_dup2(int oldfd, int newfd, int *errnum)
{
    // LOCAL VARIABLES
    int fd = SKID_BAD_FD;  // File descriptor
    int results = ENOERR;  // Errno value

    // INPUT VALIDATION
    // oldfd
    results = validate_skid_fd(oldfd);
    // newfd
    if (ENOERR == results)
    {
        results = validate_skid_fd(newfd);
    }
    // errnum
    if (ENOERR == results)
    {
        results = validate_skid_err(errnum);
    }

    // CALL IT
    if (ENOERR == results)
    {
        fd = dup2(oldfd, newfd);
        if (fd < 0)
        {
            results = errno;
            fd = SKID_BAD_FD;
            PRINT_ERROR(The call to dup2() failed);
            PRINT_ERRNO(results);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return fd;
}


int open_fd(const char *filename, int flags, mode_t mode, int *errnum)
{
    // LOCAL VARIABLES
    int fd = SKID_BAD_FD;  // File descriptor
    int results = ENOERR;  // Errno value

    // INPUT VALIDATION
    if (NULL == filename || NULL == errnum)
    {
        results = EINVAL;  // NULL pointer
    }

    // OPEN IT
    if (ENOERR == results)
    {
        fd = open(filename, flags, mode);
        if (-1 == fd)
        {
            results = errno;
            PRINT_ERROR(The call to open() failed);
            PRINT_ERRNO(results);
            fd = SKID_BAD_FD;
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return fd;
}


char *read_fd(int fd, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Errno values
    char *output_buf = NULL;  // Output buffer containing the read contents of the file descriptor
    size_t output_size = 0;   // The size of output_buf

    // INPUT VALIDATION
    if (errnum)
    {
        result = validate_skid_fd(fd);
    }
    else
    {
        result = EINVAL;  // NULL pointer
    }

    // READ IT
    if (ENOERR == result)
    {
        result = read_fd_dynamic(fd, &output_buf, &output_size);
    }

    // CLEAN UP
    if (ENOERR != result)
    {
        free_skid_mem((void **)&output_buf);
        output_size = 0;
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return output_buf;
}


int write_fd(int fd, const char *msg)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Errno values
    size_t msg_len = 0;       // Length of the msg string
    ssize_t bytes_wrote = 0;  // Number of bytes written

    // INPUT VALIDATION
    result = validate_skid_fd(fd);
    if (ENOERR == result)
    {
        result = validate_skid_string(msg, false);
    }

    // WRITE IT
    // Size it
    if (ENOERR == result)
    {
        msg_len = strlen(msg);
    }
    // Write it
    if (ENOERR == result)
    {
        errno = ENOERR;  // Clear errno
        bytes_wrote = write(fd, (void *)msg, msg_len * sizeof(char));
        if (bytes_wrote < 0)
        {
            result = errno;
            PRINT_ERROR(The call to write() failed);
            PRINT_ERRNO(result);
        }
        else if (bytes_wrote < (msg_len * sizeof(char)))
        {
            PRINT_WARNG(A partial write occurred);
            result = write_fd(fd, msg + bytes_wrote);  // Finish the write or force an error
        }
    }

    // DONE
    return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL bool check_for_space(size_t bytes_read, size_t output_len, size_t output_size)
{
    // LOCAL VARIABLES
    bool has_room = false;  // Is there enough room in the output buffer to store bytes_read

    // CHECK IT
    if (bytes_read > 0 && output_size > 0)
    {
        if ((output_size - output_len) >= bytes_read)
        {
            has_room = true;
        }
    }

    // DONE
    return has_room;
}


SKID_INTERNAL int check_for_pre_alloc(char **output_buf, size_t *output_size)
{
    // LOCAL VARIABLES
    int result = ENOERR;   // Success of execution
    char *tmp_ptr = NULL;  // Return value from allocation

    // INPUT VALIDATION
    result = validate_sfd_args(output_buf, output_size);

    // CHECK IT
    if (ENOERR == result)
    {
        if (NULL == *output_buf)
        {
            tmp_ptr = alloc_skid_mem(SKID_FD_BUFF_SIZE, sizeof(char), &result);
            if (NULL == tmp_ptr)
            {
                PRINT_ERROR(The call to alloc_skid_mem() failed);
                PRINT_ERRNO(result);
            }
            else
            {
                *output_buf = tmp_ptr;  // Store the pointer
                *output_size = SKID_FD_BUFF_SIZE;  // Update the size
            }
        }
    }

    // DONE
    return result;
}


SKID_INTERNAL int read_fd_dynamic(int fd, char **output_buf, size_t *output_size)
{
    // LOCAL VARIABLES
    int result = validate_skid_fd(fd);          // Success of execution
    char local_buf[SKID_FD_BUFF_SIZE] = { 0 };  // Local buffer
    ssize_t num_read = 0;                       // Number of bytes read
    size_t output_len = 0;                      // The length of *output_buf's string
    char *tmp_ptr = NULL;                       // Temp pointer

    // INPUT VALIDATION
    if (ENOERR == result)
    {
        result = validate_sfd_args(output_buf, output_size);
    }

    // SETUP
    if (ENOERR == result)
    {
        result = check_for_pre_alloc(output_buf, output_size);
    }

    // READ DYNAMIC
    if (ENOERR == result)
    {
        while (1)
        {
            output_len = strlen(*output_buf);  // Get the current length of output_buf
            // Read into local buff
            num_read = read(fd, local_buf, sizeof(local_buf));
            if (0 == num_read)
            {
                 FPRINTF_ERR("%s - Call to read() reached EOF\n", DEBUG_INFO_STR);
                 break;  // Done reading
            }
            // Check for room
            if (false == check_for_space(num_read, output_len, *output_size))
            {
                // Not enough room?  Reallocate.
                result = realloc_fd_dynamic(output_buf, output_size);
                if (ENOERR != result)
                {
                    PRINT_ERROR(The call to realloc_fd_dynamic() failed);
                    PRINT_ERRNO(result);
                    break;  // Stop on error
                }
            }
            // Copy local buff into *output_buf
            if (true == check_for_space(num_read, output_len, *output_size))
            {
                strncat(*output_buf, local_buf, *output_size - output_len);  // Add local to output
                memset(local_buf, 0x0, sizeof(local_buf));  // Zeroize the local buffer
            }
            else
            {
                PRINT_ERROR(Logic Failure - realloc_fd_dynamic succeeded incorrectly);
                result = EOVERFLOW;
            }
        }
    }

    // VERIFY NUL-TERMINATED
    if (ENOERR == result)
    {
        if (output_len == *output_size)
        {
            tmp_ptr = copy_skid_string(*output_buf, &result);
            if (ENOERR == result)
            {
                free_skid_mem((void **)output_buf);  // Free the old buffer
                *output_buf = tmp_ptr;  // Save the new buffer
                *output_size += 1;  // Made room for nul-termination
            }
        }
    }

    // CLEANUP
    if (ENOERR != result)
    {
        // output_buf
        free_skid_mem((void **)output_buf);
        // output_size
        if (output_size)
        {
            *output_size = 0;
        }
        // tmp_ptr
        free_skid_mem((void **)&tmp_ptr);
    }

    // DONE
    return result;
}


SKID_INTERNAL int realloc_fd_dynamic(char **output_buf, size_t *output_size)
{
    // LOCAL VARIABLES
    int result = ENOERR;   // Success of execution
    size_t new_size = 0;   // New size of the allocation
    char *tmp_ptr = NULL;  // Return value from allocation

    // INPUT VALIDATION
    result = validate_sfd_args(output_buf, output_size);
    if (ENOERR == result)
    {
        if (0 >= *output_size)
        {
            result = EINVAL;  // They should have called check_for_pre_alloc()
        }
    }

    // REALLOCATE
    // Determine new size
    if (ENOERR == result)
    {
        // Check for maximum
        if (SKID_MAX_SZ == *output_size)
        {
            result = EOVERFLOW;  // Buffer size is already at its maximum value
        }
        // Check for overflow
        else if (*output_size > (SKID_MAX_SZ - *output_size))
        {
            result = EOVERFLOW;  // Not enough room left to double it
        }
        else
        {
            new_size = 2 * (*output_size);
        }
    }
    // Allocate
    if (ENOERR == result)
    {
        tmp_ptr = alloc_skid_mem(new_size, sizeof(char), &result);
        if (NULL == tmp_ptr)
        {
            PRINT_ERROR(The call to alloc_skid_mem() failed);
            PRINT_ERRNO(result);
        }
    }
    // Copy old into new
    if (ENOERR == result)
    {
        strncpy(tmp_ptr, *output_buf, *output_size);
    }
    // Free old
    if (ENOERR == result)
    {
        result = free_skid_mem((void **)output_buf);
        *output_size = 0;  // Zero out the size
    }
    // Update out arguments
    if (ENOERR == result)
    {
        *output_buf = tmp_ptr;  // Store the pointer
        *output_size = new_size;  // Update the size
    }

    // DONE
    return result;
}


SKID_INTERNAL int validate_sfd_args(char **output_buf, size_t *output_size)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Validation result

    // VALIDATE IT
    // output_buf
    if (NULL == output_buf)
    {
        result = EINVAL;  // NULL pointer
    }
    // output_size
    else if (NULL == output_size)
    {
        result = EINVAL;  // NULL pointer
    }
    // output_buf && output_size
    else
    {
        if (NULL != *output_buf && *output_size <= 0)
        {
            result = EINVAL;  // Buffer pointer exists but size is invalid
            PRINT_ERROR(Invalid size of an existing buffer pointer);
        }
    }

    // DONE
    return result;
}
