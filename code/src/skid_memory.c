/*
 *  This library defines functionality to allocate and free memory on behalf of SKID.
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // errno
#include <stdbool.h>                        // false
#include <stdlib.h>                         // calloc()
#include <string.h>                         // strlen()
#include "skid_debug.h"                     // PRINT_ERRNO()
#include "skid_macros.h"                    // ENOERR, SKID_INTERNAL
#include "skid_memory.h"                    // public functions, skidMemMapRegion*
#include "skid_validation.h"                // validate_skid_err(), validate_skid_pathname()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Validate common arguments on behalf of skid_memory.
 *
 *  Args:
 *      pathname: A non-NULL, non-empty, pathname.
 *      err: A non-NULL integer pointer.
 *
 *  Returns:
 *      0 for good input, errno for failed validation.
 */
SKID_INTERNAL int validate_sm_standard_args(const char *pathname, int *err);

/*
 *  Description:
 *      Validate pathnames on behalf of skid_memory.
 *
 *  Args:
 *      pathname: A non-NULL, non-empty, pathname.
 *
 *  Returns:
 *      0 for good input, errno for failed validation.
 */
SKID_INTERNAL int validate_sm_pathname(const char *pathname);

/*
 *  Description:
 *      Validate skidMemMapRegion struct pointers on behalf of skid_memory.
 *
 *  Args:
 *      map_mem: A non-NULL, well-formed, struct.
 *
 *  Returns:
 *      ENOERR for good input, errno for failed validation.
 */
SKID_INTERNAL int validate_sm_struct(skidMemMapRegion_ptr map_mem);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


void *alloc_skid_mem(size_t num_elem, size_t size_elem, int *errnum)
{
    // LOCAL VARIABLES
    void *new_mem = NULL;  // Heap allocated memory
    int result = EINVAL;   // Store local errno values here

    // INPUT VALIDATION
    if (num_elem > 0 && size_elem > 0 && errnum)
    {
        result = ENOERR;  // Looks good
    }

    // ALLOCATE IT
    if (ENOERR == result)
    {
        new_mem = calloc(num_elem, size_elem);
        if (!new_mem)
        {
            result = errno;
            PRINT_ERROR(The call to calloc() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return new_mem;
}


char *copy_skid_string(const char *source, int *errnum)
{
    // LOCAL VARIABLES
    char *destination = NULL;  // Heap-allocated copy of source
    int result = ENOERR;       // Errno values
    size_t src_len = 0;        // Length of source

    // INPUT VALIDATION
    result = validate_sm_standard_args(source, errnum);

    // COPY IT
    // Size it
    if (ENOERR == result)
    {
        src_len = strlen(source);
    }
    // Allocate
    if (ENOERR == result)
    {
        destination = alloc_skid_mem(src_len + 1, sizeof(char), &result);
    }
    // Copy
    if (ENOERR == result)
    {
        strncpy(destination, source, src_len);
    }

    // CLEANUP
    if (result)
    {
        free_skid_string(&destination);  // Best effort
    }

    // DONE
    if (errnum)
    {
        *errnum = result;
    }
    return destination;
}


int free_skid_mem(void **old_mem)
{
    // LOCAL VARIABLES
    int result = EINVAL;  // Results from execution

    // INPUT VALIDATION
    if (old_mem && *old_mem)
    {
        result = ENOERR;
    }

    // FREE IT
    if (ENOERR == result)
    {
        free(*old_mem);
        *old_mem = NULL;
    }

    // DONE
    return result;
}


int free_skid_string(char **old_string)
{
    // LOCAL VARIABLES
    int result = free_skid_mem((void **)old_string);  // Results from execution

    // DONE
    return result;
}


int map_skid_mem(skidMemMapRegion_ptr new_map, int prot, int flags)
{
    // LOCAL VARIABLES
    int result = validate_sm_struct(new_map);  // Store errno value
    int new_flags = flags | MAP_ANONYMOUS;     // New flags to pass to mmap()

    // MAP IT
    if (ENOERR == result)
    {
        errno = ENOERR;  // Initialize errno... for safety
        new_map->addr = mmap(new_map->addr, new_map->length, prot, new_flags);
        if (MAP_FAILED == new_map->addr)
        {
            result = errno;  // Something failed
            PRINT_ERROR(The call to mmap failed);
            PRINT_ERRNO(result);
            new_map->addr = NULL;  // Zeroize the pointer
            new_map->length = 0;  // Reset the length
        }
    }

    // DONE
    return result;
}


int unmap_skid_mem(skidMemMapRegion_ptr old_map)
{
    // LOCAL VARIABLES
    int result = validate_sm_struct(new_map);  // Store errno value

    // UNMAP IT
    if (ENOERR == result)
    {
        errno = ENOERR;  // Initialize errno... for safety
        if (0 == munmap(old_map->addr, old_map->length))
        {
            old_map->addr = NULL;  // Zeroize the pointer
            old_map->length = 0;  // Reset the length
        }
        else
        {
            result = errno;  // Something failed
            PRINT_ERROR(The call to munmap failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL int validate_sm_standard_args(const char *pathname, int *err)
{
    // LOCAL VARIABLES
    int result = validate_sm_pathname(pathname);  // Store errno value

    // INPUT VALIDATION
    if (ENOERR == result)
    {
        result = validate_skid_err(err);
    }

    // DONE
    return result;
}


SKID_INTERNAL int validate_sm_pathname(const char *pathname)
{
    return validate_skid_pathname(pathname, false);  // Refactored for backwards compatibility
}


SKID_INTERNAL int validate_sm_struct(skidMemMapRegion_ptr map_mem)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Store errno values

    // INPUT VALIDATION
    if (NULL == map_mem)
    {
        result = EINVAL;
        PRINT_ERROR(Received an invalid pointer in map_mem);
    }
    else
    {
        if (NULL == map_mem->addr && map_mem->length > 0)
        {
            result = EINVAL;
            PRINT_ERROR(An empty pointer may not have a non-zero length);
        }
        else if (NULL != map_mem->addr && 0 == map_mem->length)
        {
            result = EINVAL;
            PRINT_ERROR(A valid pointer may not have a zero length);
        }
    }

    // DONE
    return result;
}
