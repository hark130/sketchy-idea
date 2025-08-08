/*
 *  This library defines functionality to create, remove, and parse Linux directories.
 */

// #define SKID_DEBUG                      // Enable DEBUG logging

#include "skid_debug.h"                 // PRINT_ERRNO()
#include "skid_dir_operations.h"        // _DEFAULT_SOURCE, delete_dir()
#include "skid_file_metadata_read.h"    // is_directory()
#include "skid_file_operations.h"       // delete_file()
#include "skid_macros.h"                // ENOERR, SKID_INTERNAL
#include "skid_memory.h"                // copy_skid_string(), free_skid_mem(), free_skid_string()
#include "skid_validation.h"            // validate_skid_err(), validate_skid_pathname()
#include <dirent.h>                     // closedir(), opendir(), readdir(), struct dirent
#include <errno.h>                      // errno
#include <stdint.h>                     // SIZE_MAX
#include <string.h>                     // strncpy()
#include <unistd.h>                     // link(), rmdir(), symlink()
#ifndef SKID_ARRAY_SIZE
// #define SKID_ARRAY_SIZE 1024            // Starting num of indices in read_dir_contents()'s array
#define SKID_ARRAY_SIZE 8               // Starting num of indices in read_dir_contents()'s array
#endif  /* SKID_ARRAY_SIZE */
#ifndef SKID_MAX_RETRIES
#define SKID_MAX_RETRIES 10             // Maximum number of maximum failure loops for store_dent()
#endif  /* SKID_MAX_RETRIES */

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Count the number of sequential, non-NULL entries in content_arr.  Stops counting at the
 *      first NULL or capacity has been reached.  Bad input will result in 0.
 *
 *  Args:
 *      content_arr: [Optional] Starting array of strings (which represent pathnames).  If NULL,
 *          this function will return 0.
 *      capacity: [In] Current capacity of content_arr.
 *
 *  Returns:
 *      The number of non-NULL entries in the content_arr.  0 on bad input: NULL pointer, invalid
 *      capacity value.
 */
SKID_INTERNAL size_t count_content_arr_entries(char **content_arr, size_t *capacity);

/*
 *  Description:
 *      Replicates skid_file_metadata_read's is_directory() functionality for dirent structs.
 *
 *  Args:
 *      direntp: A pointer to a dirent struct.
 *
 *  Returns:
 *      True if direntp represents a directory, false otherwise.  Also returns false on any error.
 */
SKID_INTERNAL bool is_dirent_a_dir(struct dirent *direntp);

/*
 *  Description:
 *      Allocate an array and copy in dirname/path.  This function will add a delimiter if
 *      dirname does not end with one and path does not begin with one.  It is the caller's
 *      responsiblity to free the return value.  Use free_skid_string() to free the return value.
 *
 *  Args:
 *      dirname: A directory, relative or absolute, to join path to.
 *      path: A path, relative or absolute, to join to dirname.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      A heap-allocated copy of dirname/path on success.  NULL on failure (see errnum for details).
 */
SKID_INTERNAL char *join_dir_path(const char *dirname, const char *path, int *errnum);

/*
 *  Description:
 *      Allocate a larger array, copy the string pointers in, update the capacity, and free the
 *      old array.
 *
 *  Args:
 *      content_arr: [Optional] Starting array of strings (which represent pathnames).  If NULL,
 *          this function will create the first array.
 *      capacity: [In/Out] Current capacity of content_arr.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      A larger string array on success, content_arr on failure.  Ensure errnum is checked for
 *      errno values.
 */
SKID_INTERNAL char **realloc_dir_contents(char **content_arr, size_t *capacity, int *errnum);

/*
 *  Description:
 *      Underpins read_dir_contents().  The caller is responsible for free()ing the return value.
 *
 *  Args:
 *      content_arr: [Optional] Starting array of strings (which represent pathnames).  There is no
 *          guarantee the return value will match content_arr.  If NULL, this function will
 *          allocate an array.  If content_arr is full, this function will reallocate an array
 *          and update capacity with the new size.
 *      capacity: [In/Out] A pointer to the current capacity of content_arr.  This value will be
 *          updated if the content array has to be reallocated.
 *      dirname: Absolute or relative directory to read the contents of (must exist).
 *      recurse: If true, also include all sub-dirs and their files in the array.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      A NULL-terminated array of string pointers.  Each string pointer represents one path
 *      found in dirname.  The array contains a number of indices equal to capacity.  If dirname
 *      is empty, this function will return NULL.  On failure, NULL an errnum will be set with
 *      an errno value (or -1 for an unspecified error).
 */
SKID_INTERNAL char **recurse_dir_contents(char **content_arr, size_t *capacity, const char *dirname,
                                          bool recurse, int *errnum);

/*
 *  Description:
 *      Allocate, copy, and store dirp->d_name in content_arr (unless d_name is . or ..).
 *
 *  Args:
 *      content_arr: [Optional] Starting array of strings (which represent pathnames).  There is no
 *          guarantee the return value will match content_arr.  If NULL, this function will
 *          allocate an array.  If content_arr is full, this function will reallocate an array,
 *          copy the legacy pointers, and update capacity with the new size.
 *      capacity: [In/Out] A pointer to the current capacity of content_arr.  This value will be
 *          updated if the content array has to be reallocated.
 *      dirname: [Optional] Preceding directory to prepend to the dirent->d_name.  If ommitted,
 *          just the d_name will be stored.
 *      direntp: [Optional] A pointer to a dirent struct to store in content_arr.  This function
 *          will ignore NULL direntp pointers.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to ENOERR on success.
 *
 *  Returns:
 *      A NULL-terminated array of string pointers.  Each string pointer represents one path.
 *      The last entry in the array will be a copy of dirp->d_name (if dirp is a valid pointer).
 *      The array contains a number of indices equal to capacity.  On failure, NULL an errnum
 *      will be set with an errno value (or -1 for an unspecified error).
 */
SKID_INTERNAL char **store_dirent(char **content_arr, size_t *capacity, const char *dirname,
                                  struct dirent *direntp, int *errnum);

/*
 *  Description:
 *      Validate struct dirent pointers on behalf of store_dirent().
 *
 *  Args:
 *      direntp: A non-NULL dirent struct pointer.
 *
 *  Returns:
 *      True if direntp is valid: non-NULL, d_name lenth > 0, is not a relative symbol
 *      (e.g., ".", "..").  False otherwise.
 */
SKID_INTERNAL bool validate_direntp(struct dirent *direntp);

/*
 *  Description:
 *      Validate the arguments on behalf of recurse_dir_contents().
 *
 *  Args:
 *      See recurse_dir_contents().
 *
 *  Returns:
 *      An errno value indicating the results of validation.  ENOERR on successful validation.
 */
SKID_INTERNAL int validate_rdc_args(char **content_arr, size_t *capacity, const char *dirname,
                                    bool recurse, int *errnum);

/*
 *  Description:
 *      Validates the pathname arguments on behalf of this library.
 *
 *  Args:
 *      pathname: A non-NULL pointer to a non-empty string.
 *
 *  Returns:
 *      An errno value indicating the results of validation.  ENOERR on successful validation.
 */
SKID_INTERNAL int validate_sdo_pathname(const char *pathname);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int create_dir(const char *dirname, mode_t mode)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution

    // INPUT VALIDATION
    result = validate_sdo_pathname(dirname);

    // DELETE IT
    if (ENOERR == result)
    {
        if (mkdir(dirname, mode))
        {
            result = errno;
            PRINT_ERROR(The call to mkdir() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


int delete_dir(const char *dirname)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution
    bool is_dir = false;  // Is dirname a directory that exists?

    // INPUT VALIDATION
    result = validate_sdo_pathname(dirname);
    if (ENOERR == result)
    {
        is_dir = is_directory(dirname, &result);
        if (false == is_dir && ENOERR == result)
        {
            result = ENOTDIR;  // It exists, just not as a directory
        }
    }

    // DELETE IT
    if (ENOERR == result)
    {
        if (rmdir(dirname))
        {
            result = errno;
            PRINT_ERROR(The call to rmdir() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


int destroy_dir(const char *dirname)
{
    // From readdir(3)
    // Only  the fields d_name and (as an XSI extension) d_ino are specified in POSIX.1.
    // Other than Linux, the d_type field is available mainly only on BSD systems.
    // The remaining fields are available on many, but not all systems.  Under glibc, programs
    // can check for the availability of the fields not defined in POSIX.1 by testing whether
    // the macros _DIRENT_HAVE_D_NAMLEN, _DIRENT_HAVE_D_RECLEN, _DIRENT_HAVE_D_OFF, or
    // _DIRENT_HAVE_D_TYPE are defined.

    // LOCAL VARIABLES
    int result = ENOERR;         // Results of execution
    char **dir_contents = NULL;  // Array of string pointers containing dirname's paths
    size_t capacity = 0;         // Size of dir_contents
    size_t num_paths = 0;        // Number of paths contained in dirname
    char *tmp_path = NULL;       // Temp pathname to manage

    // INPUT VALIDATION
    result = validate_sdo_pathname(dirname);

    // DESTROY IT
    // Get the full list
    if (ENOERR == result)
    {
        dir_contents = read_dir_contents(dirname, true, &result, &capacity);
        if (ENOERR != result)
        {
            PRINT_ERROR(The call to read_dir_contents() failed);
            PRINT_ERRNO(result);
        }
    }
    // Size it
    if (ENOERR == result)
    {
        num_paths = count_content_arr_entries(dir_contents, &capacity);
    }
    // Destroy it (backwards)
    if (ENOERR == result)
    {
        while (num_paths > 0)
        {
            tmp_path = dir_contents[num_paths - 1];  // Start at the end
            if (tmp_path)
            {
                if (true == is_regular_file(tmp_path, &result))
                {
                    result = delete_file(tmp_path);
                }
                else if (true == is_directory(tmp_path, &result))
                {
                    result = delete_dir(tmp_path);
                }
                else
                {
                    if (ENOERR == result)
                    {
                        PRINT_WARNG(Detected an unsupported path type);
                        FPRINTF_ERR("%s - %s is unsupported\n", DEBUG_WARNG_STR, tmp_path);
                    }
                    else
                    {
                        PRINT_ERROR(Encountered a deletion error);
                        PRINT_ERRNO(result);
                        FPRINTF_ERR("%s - Attempting to delete '%s'\n", DEBUG_ERROR_STR, tmp_path);
                        break;  // Let's stop since the rest will likely  error as well
                    }
                }
            }
            else
            {
                PRINT_WARNG(The array count appears to have been off);
                break;  // Found a NULL so we'll stop iterating
            }
            num_paths--;
        }
    }
    // Finally, remove the original dir
    if (ENOERR == result)
    {
        result = delete_dir(dirname);
    }

    // CLEANUP
    if (dir_contents)
    {
        free_skid_dir_contents(&dir_contents);  // Best effort
    }

    // DONE
    return result;
}


int free_skid_dir_contents(char ***dir_contents)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Errno value
    char **old_array = NULL;  // Pointer to the array

    // INPUT VALIDATION
    if (NULL == dir_contents)
    {
        result = EINVAL;  // NULL pointer
    }

    // FREE IT
    // Free the string pointers
    if (ENOERR == result)
    {
        old_array = *dir_contents;  // Array pointer
        while (NULL != old_array && NULL != *old_array && ENOERR == result)
        {
            result = free_skid_string(old_array);
            old_array++;
        }
    }
    // Free the array
    if (ENOERR == result)
    {
        free_skid_mem((void **)dir_contents);
    }

    // DONE
    return result;
}


char **read_dir_contents(const char *dirname, bool recurse, int *errnum, size_t *capacity)
{
    // LOCAL VARIABLES
    char **content_arr = NULL;                    // NULL-terminated array of nul-terminated strs
    int result = validate_sdo_pathname(dirname);  // Capture errno values here

    // INPUT VALIDATION
    if (ENOERR == result)
    {
        result = validate_skid_err(errnum);
    }
    if (ENOERR == result)
    {
        if (!capacity)
        {
            result = EINVAL;  // NULL pointer
        }
    }

    // READ IT
    if (ENOERR == result)
    {
        content_arr = recurse_dir_contents(content_arr, capacity, dirname, recurse, &result);
        if (ENOERR != result)
        {
            PRINT_ERROR(The intial call to recurse_dir_contents() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return content_arr;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL size_t count_content_arr_entries(char **content_arr, size_t *capacity)
{
    // LOCAL VARIABLES
    size_t count = 0;              // Number of non-NULL entries in content_arr.
    size_t curr_capacity = 0;      // Current capacity

    // INPUT VALIDATION
    if (content_arr && capacity)
    {
        curr_capacity = *capacity;
        for (size_t i = 0; i < curr_capacity; i++)
        {
            if (NULL == content_arr[i])
            {
                break;  // Found a NULL
            }
            else
            {
                count++;  // Found a pointer
            }
        }
    }

    // DONE
    return count;
}


SKID_INTERNAL bool is_dirent_a_dir(struct dirent *direntp)
{
    // LOCAL VARIABLES
    bool is_dir = false;  // Does direntp represent a directory?

    // INPUT VALIDATION
    if (direntp)
    {
        if (DT_DIR == direntp->d_type)
        {
            is_dir = true;
        }
    }

    // DONE
    return is_dir;
}


SKID_INTERNAL char *join_dir_path(const char *dirname, const char *path, int *errnum)
{
    // LOCAL VARIABLES
    char work_buff[PATH_MAX + 3] = { 0 };  // Working array
    char *new_buff = NULL;                 // Heap-allocated copy of work_buff
    int result = ENOERR;                   // Errno values

    // INPUT VALIDATION
    // dirname
    result = validate_sdo_pathname(dirname);
    // path
    if (ENOERR == result)
    {
        result = validate_sdo_pathname(path);
    }
    // errnum
    if (ENOERR == result)
    {
        result = validate_skid_err(errnum);
    }

    // JOIN IT
    // Copy dirname in
    if (ENOERR == result)
    {
        strncpy(work_buff, dirname, strlen(dirname) + 1);
    }
    // Add delimiter?
    if (ENOERR == result)
    {
        if ('/' != work_buff[strlen(work_buff)-1] && '/' != path[0])
        {
            work_buff[strlen(work_buff)] = '/';
        }
    }
    // Concatenate path
    if (ENOERR == result)
    {
        strncat(work_buff, path, (sizeof(work_buff)/sizeof(*work_buff)) - strlen(work_buff));
    }
    // Allocate and copy
    if (ENOERR == result)
    {
        new_buff = copy_skid_string(work_buff, &result);
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return new_buff;
}


SKID_INTERNAL char **realloc_dir_contents(char **content_arr, size_t *capacity, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_skid_err(errnum);  // Errno value
    char **new_array = NULL;                 // Newly allocated array
    char **old_array = content_arr;          // Old array
    size_t new_size = 0;                     // Size of the new array
    size_t curr_capacity = 0;                // Current capacity

    // INPUT VALIDATION
    if (ENOERR == result)
    {
        if (!capacity)
        {
            result = EINVAL;  // NULL pointer
        }
        else
        {
            curr_capacity = *capacity;
        }
    }

    // REALLOC IT
    // Size it
    if (ENOERR == result)
    {
        if (NULL == old_array || 0 == curr_capacity)
        {
            new_size = SKID_ARRAY_SIZE;  // Starting size
        }
        // Verify it's not already maxxed out
        else if (SIZE_MAX == curr_capacity)
        {
            result = ENOMEM;  // Already at the max
        }
        // Check for overflow
        else if (curr_capacity > (SIZE_MAX - curr_capacity))
        {
            new_size = SIZE_MAX;  // This is as big as it gets
        }
        // Double it
        else
        {
            new_size = curr_capacity * 2;
        }
    }
    // Allocate a larger array
    if (ENOERR == result)
    {
        new_array = alloc_skid_mem(new_size, sizeof(char*), &result);
    }
    // Copy the pointers in
    if (ENOERR == result && old_array)
    {
        for (size_t i = 0; i < curr_capacity; i++)
        {
            if (old_array[i])
            {
                new_array[i] = old_array[i];
            }
            else
            {
                break;  // Stop copying at the first NULL
            }
        }
    }

    // WRAP UP
    // One of the arrays has to go
    if (ENOERR == result)
    {
        free_skid_mem((void **)&old_array);  // Free the original array
        *capacity = new_size;  // Update the capacity
    }
    else
    {
        // Free the new array
        free_skid_mem((void **)&new_array);  // Best effort
        // Restore the old capacity
        if (capacity)
        {
            *capacity = curr_capacity;
        }
        new_array = old_array;  // Return the old array
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return new_array;
}


SKID_INTERNAL char **recurse_dir_contents(char **content_arr, size_t *capacity, const char *dirname,
                                          bool recurse, int *errnum)
{
    // LOCAL VARIABLES
    char **curr_arr = content_arr;      // Current content array
    int result = ENOERR;                // Errno value
    DIR *dirp = NULL;                   // Directory stream pointer
    struct dirent *temp_dirent = NULL;  // Temporary dirent struct pointer
    size_t curr_count = 0;              // Current count of content_arr entries

    // INPUT VALIDATION
    result = validate_rdc_args(content_arr, capacity, dirname, recurse, errnum);

    // RECURSION FTW!
    // Open the directory
    if (ENOERR == result)
    {
        dirp = opendir(dirname);
        if (NULL == dirp)
        {
            result = errno;
            PRINT_ERROR(The call to opendir() failed);
            PRINT_ERRNO(result);
        }
    }
    // Read the directory
    if (ENOERR == result)
    {
        while (1)
        {
            // To distinguish end of stream from an error, set errno to zero before calling
            // readdir() and then check the value of errno if NULL is returned.
            errno = ENOERR;  // Clear errno... see: readdir(3)
            temp_dirent = readdir(dirp);
            if (NULL == temp_dirent)
            {
                result = errno;
                if (ENOERR != result)
                {
                    PRINT_ERROR(The call to readdir() failed);
                    PRINT_ERRNO(result);
                }
                else
                {
                    break;  // The end of the directory stream has been reached
                }
            }
            else
            {
                // Store it
                curr_arr = store_dirent(curr_arr, capacity, dirname, temp_dirent, &result);
                if (ENOERR != result)
                {
                    PRINT_ERROR(The call to store_dirent() failed);
                    PRINT_ERRNO(result);
                    break;
                }
                // Should we recurse on this valid directory?
                if (true == recurse && validate_direntp(temp_dirent) \
                    && true == is_dirent_a_dir(temp_dirent))
                {
                    // Recurse!
                    curr_count = count_content_arr_entries(curr_arr, capacity);
                    if (curr_count >= 1)
                    {
                        curr_arr = recurse_dir_contents(curr_arr, capacity, curr_arr[curr_count-1],
                                                        recurse, &result);
                    }
                    else
                    {
                        PRINT_ERROR(Detected failed recursion in recurse_dir_contents());
                    }
                }
            }
        }
    }

    // CLEANUP
    // Close the directory stream pointer
    if (dirp)
    {
        if (closedir(dirp))
        {
            if (ENOERR == result)
            {
                result = errno;
                PRINT_ERROR(The call to closedir() failed);
                PRINT_ERRNO(result);
            }
        }
    }
    // Free any existing array on an error
    if (ENOERR != result)
    {
        free_skid_dir_contents(&curr_arr);  // Best effort
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return curr_arr;
}


SKID_INTERNAL char **store_dirent(char **content_arr, size_t *capacity, const char *dirname,
                                  struct dirent *direntp, int *errnum)
{
    // LOCAL VARIABLES
    char **curr_arr = content_arr;  // Current content array
    int result = ENOERR;            // Errno value
    int num_loops = 0;              // Maximum number of loops before direntp is stored
    size_t curr_capacity = 0;       // Current capacity
    size_t num_entries = 0;         // Number of entries already in content_arr

    // INPUT VALIDATION
    if (!capacity || !errnum)
    {
        result = EINVAL;  // Bad input
    }
    // STORE IT
    else if (direntp)
    {
        if (true == validate_direntp(direntp))
        {
            while (num_loops <= SKID_MAX_RETRIES)
            {
                // Gotta store this dirent
                // 1. Is there are an array?
                if (curr_arr)
                {
                    curr_capacity = *capacity;  // Get the current capacity
                    // 2. If so, is there space?
                    num_entries = count_content_arr_entries(curr_arr, capacity);
                    if (num_entries < (curr_capacity - 1))
                    {
                        // 3. Allocate, copy, and store it
                        if (ENOERR == validate_sdo_pathname(dirname))
                        {
                            curr_arr[num_entries] = join_dir_path(dirname, direntp->d_name,
                                                                  &result);
                        }
                        else
                        {
                            curr_arr[num_entries] = copy_skid_string(direntp->d_name, &result);
                        }
                        // 4. Verify
                        if (ENOERR != result)
                        {
                            PRINT_ERROR(The direntp->d_name copy operation failed);
                            PRINT_ERRNO(result);
                            break;  // The copy failed
                        }
                        else
                        {
                            break;  // We stored it.  Done.
                        }
                    }
                    // Then make a bigger one
                    else
                    {

                        curr_arr = realloc_dir_contents(curr_arr, capacity, &result);
                        if (ENOERR != result)
                        {
                            PRINT_ERROR(The call to realloc_dir_contents() failed);
                            PRINT_ERRNO(result);
                            break;  // The reallocation failed
                        }
                        else
                        {
                            FPRINTF_ERR("%s - %s - %s() - %d - resized the dir contents array!\n",
                                        DEBUG_INFO_STR, __FILE__, __FUNCTION_NAME__, __LINE__);
                        }
                    }
                }
                // Then make one
                else
                {
                    curr_arr = realloc_dir_contents(curr_arr, capacity, &result);
                    if (ENOERR != result)
                    {
                        PRINT_ERROR(The initial call to realloc_dir_contents() failed?);
                        PRINT_ERRNO(result);
                        break;  // The reallocation failed
                    }
                }

                num_loops++;  // One loop completed without success
            }
            // Verify we did something
            if (ENOERR == result && num_loops > SKID_MAX_RETRIES)
            {
                result = -1;  // It appears we exceeded the max loops without succeeding or failing
            }
        }
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return curr_arr;
}


SKID_INTERNAL bool validate_direntp(struct dirent *direntp)
{
    // LOCAL VARIABLES
    bool is_valid = true;  // Is direntp valid?

    // INPUT VALIDATION
    if (!direntp)
    {
        is_valid = false;  // NULL pointer
    }
    else if (0 >= strlen(direntp->d_name))
    {
        is_valid = false;  // Empty string
    }
    else if (!strcmp(".", direntp->d_name))
    {
        is_valid = false;  // We will not store this
    }
    else if (!strcmp("..", direntp->d_name))
    {
        is_valid = false;  // We will not store this either
    }

    // DONE
    return is_valid;
}


SKID_INTERNAL int validate_rdc_args(char **content_arr, size_t *capacity, const char *dirname,
                                    bool recurse, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_sdo_pathname(dirname);  // The results of validation

    // INPUT VALIDATION
    // Skipping content_arr
    // capacity
    if (ENOERR == result)
    {
        if (!capacity)
        {
            result = EINVAL;  // NULL pointer
        }
    }
    // dirname already validated
    // errnum
    if (ENOERR == result)
    {
        result = validate_skid_err(errnum);
    }

    // DONE
    return result;
}


SKID_INTERNAL int validate_sdo_pathname(const char *pathname)
{
    return validate_skid_pathname(pathname, false);  // Refactored for backwards compatibility
}
