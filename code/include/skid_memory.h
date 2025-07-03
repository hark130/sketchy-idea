/*
 *  This library defines functionality to allocate and free memory on behalf of SKID.
 *  Macros have been implemented to assist with Resource Acquisition Is Initialization (RAII) style
 *  automated cleanup of skid_memory allocations.
 *
 *  USAGE:
 *      // These macros utilize the gcc attribute "cleanup".  Their usage will fail compilation
 *      // if the attribute isn't supported.  Also, don't use these macros on non-skid_memory
 *      // functions (e.g., calloc(), malloc()).  Most importantly, the function containing the
 *      // variable must leave its code block (AKA return) before the program is terminated
 *      // (AKA exit).  TL;DR - Don't call exit() in the same code block as this macro.
 *      int errnum = ENOERR;  // Out-parameter for the results of SKID API functions
 *
 *      SKID_AUTO_FREE_CHAR char *string = copy_skid_string("My string", &errnum);  // A string var
 *      // Utilize string, as normal
 *      // string is automatically free()'d when it goes out of scope
 *
 *      SKID_AUTO_FREE_VOID void *buffer = alloc_skid_mem(128, 8, &errnum);  // A buffer var
 *      // Utilize buffer, as normal
 *      // buffer is automatically free()'d when it goes out of scope
 */

#ifndef __SKID_MEMORY__
#define __SKID_MEMORY__

#if (defined(__GNUC__) || defined(__clang__))
// Auto-cleanup a "char *" variable which holds a heap-allocated address provided by skid_memory.
#define SKID_AUTO_FREE_CHAR __attribute__((cleanup(free_skid_string)))  // char *-use only
// Auto-cleanup a "void *" variable which holds a heap-allocated address provided by skid_memory.
#define SKID_AUTO_FREE_VOID __attribute__((cleanup(free_skid_mem)))     // void *-use only
#else
// Let the compiler inform the user that these macros are unresolved.
//  Otherwise, defining these macros as "empty" (which is normally standard) will likely result in
//  a memory leak and that BUG shouldn't pass quietly.
// An alternative was to use #error to invoke a pre-processor phase error message but I didn't
//  want to penalize the user for utilizing a different compiler.
#endif  /* SKID_AUTO_FREE_CHAR, SKID_AUTO_FREE_VOID */

#include <stddef.h>                         // size_t
#include <sys/mman.h>                       // mmap() prot and flag macros
#include "skid_macros.h"                    // ENOERR

// This struct communicates details about mapped memory to map_skid_mem() and unmap_skid_mem().
typedef struct _skidMemMapRegion
{
    char *addr;     // [In/Out] Pointer to the virtual address space mapping
    size_t length;  // [Out] The length of the mapping
} skidMemMapRegion, *skidMemMapRegion_ptr;

/*
 *  Description:
 *      Allocate a zeroized array in heap memory.
 *
 *  Args:
 *      num_elem: The number of elements in the array.
 *      size_elem: The size of each element in the array.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated memory of total size num_elem * size_elem that has been zeroized, on success.
 *      Caller is responsible for freeing the return value with free_skid_mem().
 *      NULL on error (check errnum for details).
 */
void *alloc_skid_mem(size_t num_elem, size_t size_elem, int *errnum);

/*
 *  Description:
 *      Determine the length of source, allocate a zeroized array in heap memory, and copy it.
 *      It is the caller's responsibility to free the return value using free_skid_string().
 *
 *  Args:
 *      source: A string to copy.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      Heap-allocated memory containing a copy of source on succes.  NULL on error (check
 *      errnum for details).
 */
char *copy_skid_string(const char *source, int *errnum);

/*
 *  Description:
 *      Free skid-allocated heap memory and set the original pointer to NULL.
 *
 *  Args:
 *      old_mem: Pointer to the heap-allocated memory's storage location.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int free_skid_mem(void **old_mem);

/*
 *  Description:
 *      Free skid-allocated string and set the original pointer to NULL.
 *
 *  Args:
 *      old_string: Pointer to the heap-allocated string's storage location.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int free_skid_string(char **old_string);

/*
 *  Description:
 *      Map zeroized virtual memory by utilizing mmap().
 *
 *  Args:
 *      new_map: [In/Out] skidMemMapRegion pointer for a new mapping.  The mapping.addr value will
 *          be passed to mmap() as a hint about where to place the mapping.  If NULL, the kernel
 *          chooses the address.  Regardless, this value will be overwritten by this function with
 *          the return value from mmap().
 *      prot: The desired memory protection of the mapping (see: mmap(2)).
 *      flags: Determines, among other things, whether updates to the mapping are visible to
 *          other processes mapping the same region.  This value will be ORed with MAP_ANONYMOUS to
 *          ensure the memory region its contents are intialized to zero.  (see: mmap(2) for
 *          additional flags)
 *
 *  Returns:
 *      ENOERR on success, errno value on error.
 */
int map_skid_mem(skidMemMapRegion_ptr new_map, int prot, int flags);

/*
 *  Description:
 *      Map zeroized virtual memory to contain the struct and an addr pointer of size length.
 *      The same mapping contains the addr region and the struct so utilize unmap_skid_struct()
 *      to delete the mapping: struct, addr, and all.
 *
 *  Args:
 *      new_struct: [Out] skidMemMapRegion_ptr pointer to hold a newly mapped struct.  The struct's
 *          addr pointer will be of length bytes that has been initialized to zero.
 *      prot: The desired memory protection of the mapping (see: mmap(2)).
 *      flags: Determines, among other things, whether updates to the mapping are visible to
 *          other processes mapping the same region.  This value will be ORed with MAP_ANONYMOUS to
 *          ensure the memory region its contents are intialized to zero.  (see: mmap(2) for
 *          additional flags)
 *      length: The size of the addr mapping in bytes.
 *
 *  Returns:
 *      ENOERR on success, errno value on error.
 */
int map_skid_struct(skidMemMapRegion_ptr *new_struct, int prot, int flags, size_t length);

/*
 *  Description:
 *      Delete the mapping for the specified address range by utilizing munmap().
 *
 *  Args:
 *      old_map: [In/Out] skidMemMapRegion pointer for a mapping  to delete.  On success, these
 *          values will be zeroized.
 *
 *  Returns:
 *      ENOERR on success, errno value on error.
 */
int unmap_skid_mem(skidMemMapRegion_ptr old_map);

/*
 *  Description:
 *      Delete the complete mapping of a struct that was mapped with map_skid_struct().
 *      Unmaps both the struct and the memory region pointed to by the addr member.
 *
 *  Args:
 *      old_struct: [In/Out] skidMemMapRegion_ptr pointer for a struct to delete.  On success, the
 *          old_struct pointer will be set to NULL.
 *
 *  Returns:
 *      ENOERR on success, errno value on error.
 */
int unmap_skid_struct(skidMemMapRegion_ptr *old_struct);

#endif  /* __SKID_MEMORY__ */
