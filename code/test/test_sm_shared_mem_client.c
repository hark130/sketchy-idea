/*
 *  Manually test skid_memory's *_shared_mem()-related functionality.
 *
 *  This manual test code performs the following actions:
 *  1. Validates input
 *  2. Opens the named semaphore
 *  2. Opens the shared memory object
 *  3. Writes a message (safely)
 *
 *  Copy/paste the following...

./code/dist/test_sm_shared_mem_client.bin

 *
 */

// Standard includes
#include <errno.h>                          // EINVAL
#include <fcntl.h>                          // O_CREAT
#include <semaphore.h>                      // sem_init(), sem_post(), sem_wait()
#include <stdbool.h>                        // false
#include <stdio.h>                          // fprintf()
#include <unistd.h>                         // sleep()
// Local includes
#define SKID_DEBUG                          // The DEBUG output is doing double duty as test output
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // map mem funcs, *_shared_mem()
#include "skid_semaphores.h"                // close_named_sem(), open_named_sem(), release_sem()
#include "skid_validation.h"                // validate_skid_*()
#include "test_sm_shared_mem.h"             // Common shared memory/semaphore macros


/*
 *  Open a POSIX shared memory object.
 */
int open_shm_obj(const char *name, int *errnum);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Release a semaphore.
 */
int release_semaphore(sem_t *semaphore);

/*
 *  Validates a shared object name.
 */
int validate_shared_object_name(const char *name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int results = ENOERR;                    // Store errno and/or results here
    /* Semaphore variables */
    char *sem_name = SEM_NAME;               // Named semaphore
    sem_t *sem_ptr = NULL;                   // Pointer to the named semaphore
    /* POSIX Shared Memory variables */
    size_t buf_size = BUF_SIZE;              // Size of the mapped memory buffer
    char *sh_mem_name = SHM_NAME;            // Shared memory object name
    int sh_mem_fd = SKID_BAD_FD;             // Shared memory object file descriptor
    skidMemMapRegion virt_mem;               // Virtual memory mapping and its metadata
    int virt_prot = PROT_READ | PROT_WRITE;  // Virtual mapped memory protection
    int virt_flags = MAP_SHARED;             // Virtual mapped memory flags
    bool wrote_it = false;                   // Did we write our message to the shared memory?

    // INPUT VALIDATION
    // CLI args
    if (argc != 1)
    {
        results = EINVAL;
    }
    // Shared memory name
    if (ENOERR == results)
    {
        results = validate_shared_object_name(sh_mem_name);
    }
    // Semaphore name
    if (ENOERR == results)
    {
        results = validate_shared_object_name(sem_name);
    }
    // Was it good?
    if (ENOERR != results)
    {
        print_usage(argv[0]);
    }

    // SETUP
    // Open the named semaphore
    if (ENOERR == results)
    {
        sem_ptr = open_named_sem(sem_name, 0, &results);  // Open the named semaphore
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to open_named_sem() failed);
            PRINT_ERRNO(results);
        }
    }
    // Open the shared memory object
    if (ENOERR == results)
    {
        sh_mem_fd = open_shm_obj(sh_mem_name, &results);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to open_shm_obj() failed);
            PRINT_ERRNO(results);
        }
    }
    // Map the virtual memory
    if (ENOERR == results)
    {
        virt_mem.addr = NULL;  // Let the kernel decide
        virt_mem.length = buf_size;  // Size of the mapped buffer
        results = map_skid_mem_fd(&virt_mem, virt_prot, virt_flags, sh_mem_fd, 0);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to map_skid_mem_fd() failed);
            PRINT_ERRNO(results);
        }
    }

    // DO IT
    while (ENOERR == results)
    {
        // Obtain the semaphore
        if (0 != sem_wait(sem_ptr))
        {
            results = errno;
            PRINT_ERROR(The call to sem_wait() failed);
            PRINT_ERRNO(results);
        }
        // Check if the shared memory is empty
        if (0 == strlen((char *)virt_mem.addr))
        {
            // Write it
            strncpy(virt_mem.addr, "Hello from the client!\n", virt_mem.length);
            wrote_it = true;
        }
        // Release the semaphore
        results = release_semaphore(sem_ptr);
        if (true == wrote_it)
        {
            break;  // Done
        }
        else
        {
            sleep(1);  // Avoid the thrash
        }
    }

    // CLEANUP
    // Mapped memory
    if (NULL != virt_mem.addr)
    {
        unmap_skid_mem(&virt_mem);  // Best effort
    }
    // Shared memory object
    if (SKID_BAD_FD != sh_mem_fd)
    {
        close_shared_mem(&sh_mem_fd, true);  // Best effort
    }
    // Named semaphore
    if (NULL != sem_ptr)
    {
        close_named_sem(&sem_ptr);  // Best effort
    }

    // DONE
    return results;
}


int open_shm_obj(const char *name, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_skid_err(errnum);  // Errno values
    int flags = O_RDWR;                      // shm_open() flags
    int shmfd = SKID_BAD_FD;                 // Shared memory object file descriptor
    size_t size = BUF_SIZE;                  // Size
    mode_t mode = SHM_MODE;                  // Permission bits for the shared memory object
    bool truncate = false;                   // It's not new so don't call ftruncate()

    // CREATE IT
    if (ENOERR == result)
    {
        shmfd = open_shared_mem(name, flags, mode, size, truncate, &result);
        if (ENOERR != result)
        {
            PRINT_ERROR(The call to open_shared_mem() failed);
            PRINT_ERRNO(result);
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return shmfd;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s\n", prog_name);
}


int release_semaphore(sem_t *semaphore)
{
    // LOCAL VARIABLES
    int results = release_sem(semaphore);

    // DONE
    return results;
}


int validate_shared_object_name(const char *name)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values

    // INPUT VALIDATION
    result = validate_skid_string(name, false);
    if (ENOERR == result)
    {
        if ('/' != name[0])
        {
            result = EINVAL;
            fprintf(stderr,
                    "%s The shared object name '%s' must begin with a '/' (for portability)\n",
                    DEBUG_ERROR_STR, name);
        }
    }

    // DONE
    return result;
}
