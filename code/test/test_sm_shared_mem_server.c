/*
 *  Manually test skid_memory's *_shared_mem()-related functionality.
 *
 *  This manual test code performs the following actions:
 *  1. Validate input
 *  2. Creates a shared memory object
 *  3. Map virtual memory for communication
 *
 *  Copy/paste the following...

./code/dist/test_sm_shared_mem_server.bin </NEW_SHARED_MEM_OBJECT> </NEW_NAMED_SEMAPHORE>

 *
 */

// Standard includes
#include <errno.h>                          // EINVAL
#include <fcntl.h>                          // O_CREAT
// #include <semaphore.h>                      // sem_init(), sem_post(), sem_trywait()
// #include <stdbool.h>                        // false
// #include <stdint.h>                         // SIZE_MAX
#include <stdio.h>                          // fprintf()
// // #include <stdlib.h>                 // exit()
// #include <sys/wait.h>                       // waitpid()
// #include <unistd.h>                         // fork()
// Local includes
#define SKID_DEBUG                          // The DEBUG output is doing double duty as test output
// // #include "devops_code.h"            // call_sigqueue()
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // map mem funcs, *_shared_mem()
// #include "skid_file_metadata_read.h"        // is_regular_file()
// #include "skid_file_operations.h"           // read_file()
// #include "skid_signal_handlers.h"   // handle_ext_read_queue_int()
// #include "skid_signals.h"           // set_signal_handler_ext(), translate_signal_code()
#include "skid_validation.h"                // validate_skid_*()

#define BUF_LEN 1024                       // Length of the mapped memory buffer

/*
 *  Create a new POSIX shared memory object
 */
int create_new_shm_obj(const char *name, size_t size, int *errnum);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Validates a shared object name.
 */
int validate_shared_object_name(const char *name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int results = ENOERR;                      // Store errno and/or results here
    size_t buf_size = BUF_LEN * sizeof(char);  // Size of the mapped memory buffer
    char *sh_mem_name = NULL;                  // Shared memory object name
    int sh_mem_fd = SKID_BAD_FD;               // Shared memory object file descriptor
    char *sem_name = NULL;                     // Named semaphore
    skidMemMapRegion virt_mem;                 // Virtual memory mapping and its metadata
    int virt_prot = PROT_READ | PROT_WRITE;    // Virtual mapped memory protection
    int virt_flags = MAP_SHARED;               // Virtual mapped memory flags

    // INPUT VALIDATION
    // CLI args
    if (argc != 3)
    {
        results = EINVAL;
    }
    else
    {
        sh_mem_name = argv[1];
        sem_name = argv[2];
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
    // Create the named semaphore
    if (ENOERR == results)
    {
        
    }
    // Create the shared memory object
    if (ENOERR == results)
    {
        sh_mem_fd = create_new_shm_obj(sh_mem_name, buf_size + 1, &results);
    }
    // Map zeroized virtual memory into the shared memory file descriptor
    if (ENOERR == results)
    {
        virt_mem.addr = NULL;  // Let the kernel decide
        virt_mem.length = buf_size + 1;  // Size of the mapped buffer
        results = map_skid_mem_fd(&virt_mem, virt_prot, virt_flags, sh_mem_fd, 0);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to map_skid_mem_fd() failed);
            PRINT_ERRNO(results);
        }
    }

    // DO IT
    // Release the semaphore
    // Check for input
    // Read it
    // Clear it
    if (ENOERR == results)
    {
        
    }

    // CLEANUP
    // Shared memory object
    if (SKID_BAD_FD != sh_mem_fd)
    {
        close_shared_mem(&sh_mem_fd, true);  // Best effort
    }
    if (NULL != sh_mem_name)
    {
        delete_shared_mem(sh_mem_name);  // Best effort
    }
    // Named semaphore


    // DONE
    return results;
}


int create_new_shm_obj(const char *name, size_t size, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_skid_err(errnum);  // Errno values
    int flags = O_CREAT | O_RDWR;            // shm_open() flags
    int shmfd = SKID_BAD_FD;                 // Shared memory object file descriptor
    // Permission bits for the shared memory object
    mode_t mode = SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | \
                  SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | \
                  SKID_MODE_OTHER_R | SKID_MODE_OTHER_W;

    // CREATE IT
    if (ENOERR == result)
    {
        shmfd = open_shared_mem(name, flags, mode, size, &result);
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
    fprintf(stderr, "Usage: %s </NEW_SHARED_MEM_OBJECT>\n", prog_name);
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
