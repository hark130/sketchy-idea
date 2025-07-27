/*
 *  This library defines functionality to allocate and free memory on behalf of SKID.
 */

#define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // errno
#include <stdbool.h>                        // false
#include <stdlib.h>                         // calloc()
#include <string.h>                         // strlen()
#include <unistd.h>                         // ftruncate()
#include "skid_debug.h"                     // PRINT_ERROR(), PRINT_ERRNO()
#include "skid_file_descriptors.h"          // close_fd()
#include "skid_macros.h"                    // ENOERR, SKID_INTERNAL
#include "skid_memory.h"                    // public functions, skidMemMapRegion*
#include "skid_validation.h"                // validate_skid_*()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Standardize the way mmap() is called and responds to errors.
 *
 *  Args:
 *      addr: [Optional] If addr is NULL, then the kernel chooses the (page-aligned) address at
 *          which to create the mapping; this is the most portable method of creating a new
 *          mapping.  If addr is not NULL, then the kernel takes it as a hint about where to
 *          place the mapping; on Linux, the kernel will pick a nearby page boundary
 *          (but always above or equal to the value specified by /proc/sys/vm/mmap_min_addr)
 *          and attempt to create the mapping there.
 *      length: The number of bytes to intiialize in a file-mapping, starting at offset.
 *      prot: Describes the desired memory protection of the mapping (and must not conflict with
 *          the open mode of the file).  It is either PROT_NONE or the bitwise OR of one or more
 *          flags (see: mmap(2)).
 *      flags: Determines whether updates to the mapping are visible to other processes
 *          mapping the same region, and whether updates are carried through to the underlying
 *          file (see: mmap(2))
 *      fd: [Optional] A file descritptor to a file mapping (or other object).
 *      offset: [Optional] Beginning of the initialization of fd.  Must be a multiple of the
 *          page size as returned by sysconf(_SC_PAGE_SIZE).
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      A pointer to the mapped area on success.  NULL on failure and errnum is set with an
 *      errno value.
 */
SKID_INTERNAL void *call_mmap(void *addr, size_t length, int prot, int flags,
                              int fd, off_t offset, int *errnum);

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
 *      mapping: If True, relaxes map_mem validation with the consideration that the struct
 *          is being used to map memory.
 *
 *  Returns:
 *      ENOERR for good input, errno for failed validation.
 */
SKID_INTERNAL int validate_sm_struct(skidMemMapRegion_ptr map_mem, bool mapping);


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


int close_shared_mem(int *shmfd, bool quiet)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values

    // INPUT VALIDATION
    // Handled by close_fd()

    // CLOSE IT
    result = close_fd(shmfd, quiet);

    // DONE
    return result;
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


int delete_shared_mem(const char *name)
{
    // LOCAL VARIABLES
    int result = validate_skid_string(name, false);  // Results from execution

    // DELETE IT
    if (ENOERR == result)
    {
        if (0 != shm_unlink(name))
        {
            result = errno;
            PRINT_ERROR(The call to shm_unlink() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
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
    int result = validate_sm_struct(new_map, true);  // Store errno value
    int new_flags = flags | MAP_ANONYMOUS;           // New flags to pass to call_mmap()

    // MAP IT
    if (ENOERR == result)
    {
        new_map->addr = call_mmap(new_map->addr, new_map->length, prot, new_flags, -1, 0, &result);
        if (ENOERR != result)
        {
            PRINT_ERROR(The call to call_mmap() failed);
            PRINT_ERRNO(result);
            new_map->addr = NULL;  // Zeroize the pointer
            new_map->length = 0;  // Reset the length
        }
    }

    // DONE
    return result;
}


int map_skid_mem_fd(skidMemMapRegion_ptr new_map, int prot, int flags, int fd, off_t offset)
{
    // LOCAL VARIABLES
    int result = validate_sm_struct(new_map, true);  // Store errno value
    int new_flags = flags;                           // New flags to pass to call_mmap()

    // MAP IT
    if (ENOERR == result)
    {
        new_map->addr = call_mmap(new_map->addr, new_map->length, prot,
                                  new_flags, fd, offset, &result);
        if (ENOERR != result)
        {
            PRINT_ERROR(The call to call_mmap() failed);
            PRINT_ERRNO(result);
            new_map->addr = NULL;  // Zeroize the pointer
            new_map->length = 0;  // Reset the length
        }
    }

    // DONE
    return result;
}


int map_skid_struct(skidMemMapRegion_ptr *new_struct, int prot, int flags, size_t length)
{
    // LOCAL VARIABLES
    int result = ENOERR;                                   // Store errno value
    size_t total_len = length + sizeof(skidMemMapRegion);  // Total size of the mapping
    skidMemMapRegion local_map;                            // Local struct

    // INPUT VALIDATION
    if (NULL == new_struct)
    {
        result = EINVAL;
        PRINT_ERROR(The new_stuct pointer may not be NULL);
    }
    else if (NULL != *new_struct)
    {
        result = EINVAL;
        PRINT_ERROR(The new_stuct pointer value is not empty);
    }
    else if (length < 1)
    {
        result = EINVAL;
        PRINT_ERROR(Invalid length of a mapping);
    }

    // MAP IT
    // Map everything
    if (ENOERR == result)
    {
        local_map.addr = NULL;
        local_map.length = sizeof(total_len);
        result = map_skid_mem(&local_map, prot, flags);
    }
    // Update the out parameter
    if (ENOERR == result)
    {
        // The beginning of the mapping holds the struct
        *new_struct = (skidMemMapRegion_ptr)local_map.addr;
        // The remainder of the mapping is for the addr portion of size length
        (*new_struct)->addr = local_map.addr + sizeof(skidMemMapRegion);
        (*new_struct)->length = length;  // The mapping is larger than this, but not addr
    }

    // DONE
    return result;
}


int open_shared_mem(const char *name, int flags, mode_t mode, size_t size, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;      // Results of execution
    int shmfd = SKID_BAD_FD;  // Shared memory object file descriptor

    // INPUT VALIDATION
    result = validate_skid_string(name, false);
    if (ENOERR == result)
    {
        if (0 >= size)
        {
            result = EINVAL;  // Invalid size for a mapping
        }
    }
    if (ENOERR == result)
    {
        result = validate_skid_err(errnum);
    }

    // OPEN IT
    // Open it
    if (ENOERR == result)
    {
        shmfd = shm_open(name, flags, mode);
        if (shmfd < 0)
        {
            result = errno;
            PRINT_ERROR(The call to shm_open() failed);
            PRINT_ERRNO(result);
            shmfd = SKID_BAD_FD;
        }
    }
    // Truncate it
    if (ENOERR == result)
    {
        if (0 != ftruncate(shmfd, size))
        {
            result = errno;
            PRINT_ERROR(The call to ftruncate() failed);
            PRINT_ERRNO(result);
            close_shared_mem(&shmfd, true);  // Best effort
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return shmfd;
}


int unmap_skid_mem(skidMemMapRegion_ptr old_map)
{
    // LOCAL VARIABLES
    int result = validate_sm_struct(old_map, false);  // Store errno value

    // UNMAP IT
    if (ENOERR == result && NULL != old_map->addr)
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
            PRINT_ERROR(The call to munmap() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    return result;
}


int unmap_skid_struct(skidMemMapRegion_ptr *old_struct)
{
    // LOCAL VARIABLES
    int result = ENOERR;         // Store errno value
    skidMemMapRegion local_map;  // Local struct

    // INPUT VALIDATION
    if (NULL == old_struct)
    {
        result = EINVAL;
        PRINT_ERROR(The old_struct pointer may not be NULL);
    }
    else
    {
        result = validate_sm_struct(*old_struct, false);
    }

    // UNMAP IT
    if (ENOERR == result)
    {
        local_map.addr = *old_struct;
        local_map.length = (*old_struct)->length + sizeof(skidMemMapRegion);
        result = unmap_skid_mem(&local_map);
    }
    if (ENOERR == result)
    {
        *old_struct = NULL;
    }

    // DONE
    return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL void *call_mmap(void *addr, size_t length, int prot, int flags,
                              int fd, off_t offset, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;   // Store errno value
    void *map_ptr = NULL;  // Pointer to the mapped area

    // INPUT VALIDATION
    result = validate_skid_err(errnum);

    if (ENOERR == result)
    {
        errno = ENOERR;  // Initialize errno... for safety
        map_ptr = mmap(addr, length, prot, flags, fd, offset);
        if (MAP_FAILED == map_ptr || NULL == map_ptr)
        {
            result = errno;
            map_ptr = NULL;  // Who returns (void *)-1 anyway?!
            PRINT_ERROR(The call to mmap() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return map_ptr;
}


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


SKID_INTERNAL int validate_sm_struct(skidMemMapRegion_ptr map_mem, bool mapping)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Store errno values

    // INPUT VALIDATION
    if (NULL == map_mem)
    {
        result = EINVAL;
        PRINT_ERROR(Received an invalid pointer in map_mem);
    }
    else if (NULL != map_mem->addr && 0 == map_mem->length)
    {
        result = EINVAL;
        PRINT_ERROR(A valid pointer may not have a zero length);
    }
    else if (false == mapping && NULL == map_mem->addr && map_mem->length > 0)
    {
        result = EINVAL;
        PRINT_ERROR(An empty pointer may not have a non-zero length outside of mapping);
    }

    // DONE
    return result;
}
