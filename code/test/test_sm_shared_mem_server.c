/*
 *  Manually test skid_memory's *_shared_mem()-related functionality.
 *
 *  This manual test code performs the following actions:
 *  1. Validates input
 *  2. Creates a shared memory object
 *  3. Map virtual memory for communication
 *
 *  Copy/paste the following...

./code/dist/test_sm_shared_mem_server.bin

 *
 */

// Standard includes
#include <errno.h>                          // EINVAL
#include <fcntl.h>                          // O_CREAT
#include <semaphore.h>                      // sem_init(), sem_post(), sem_trywait()
#include <stdint.h>                         // intmax_t
#include <stdio.h>                          // fprintf()
#include <unistd.h>                         // sleep()
#define SKID_DEBUG                          // The DEBUG output is doing double duty as test output
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_file_descriptors.h"          // close_fd()
#include "skid_file_operations.h"           // delete_file()
#include "skid_macros.h"                    // ENOERR
#include "skid_memory.h"                    // map mem funcs, *_shared_mem()
#include "skid_signal_handlers.h"           // handle_signal_number()
#include "skid_signals.h"                   // set_signal_handler()
#include "skid_validation.h"                // validate_skid_*()
#include "test_sm_shared_mem.h"             // Common shared memory/semaphore macros

#define SHUTDOWN_SIG SIGINT                 // "Shutdown" signal

/*
 *  Create a new POSIX shared memory object.
 */
int create_new_shm_obj(const char *name, size_t size, int *errnum);

/*
 *  Print instructions: shared memory object, named semaphore, shutdown.
 */
void print_shutdown(const char *prog_name, const char *shm_path, const char *sem_path);

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
    size_t sem_size = sizeof(sem_t);         // SPOT for the size of the semaphore
    /* POSIX Shared Memory variables */
    size_t buf_size = BUF_SIZE;              // Size of the mapped memory buffer
    char *sh_mem_name = SHM_NAME;            // Shared memory object name
    int sh_mem_fd = SKID_BAD_FD;             // Shared memory object file descriptor
    skidMemMapRegion virt_mem;               // Virtual memory mapping and its metadata
    int prot = PROT_READ | PROT_WRITE;       // mmap() protections
    int flags = MAP_SHARED;                  // mmap() flags

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
    // Signal Handler
    if (ENOERR == results)
    {
        results = set_signal_handler(SHUTDOWN_SIG, handle_signal_number, 0, NULL);
    }
    // Create the named semaphore
    if (ENOERR == results)
    {
        sem_ptr = create_named_sem(sem_name, 0, SEM_MODE, &results);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to create_named_sem() failed);
            PRINT_ERRNO(results);
        }
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
        results = map_skid_mem_fd(&virt_mem, prot, flags, sh_mem_fd, 0);
        if (ENOERR != results)
        {
            PRINT_ERROR(The call to map_skid_mem_fd() failed on the buffer);
            PRINT_ERRNO(results);
        }
    }

    // DO IT
    // Release the semaphore
    if (ENOERR == results)
    {
        results = release_semaphore((sem_t *)sem_mem.addr);
    }
    // Print instructions
    if (ENOERR == results)
    {
        print_shutdown(argv[0], sh_mem_name, sem_name);
    }
    // Check for input
    while (ENOERR == results && SHUTDOWN_SIG != skid_sig_hand_signum)
    {
        // Obtain the lock
        while (-1 == sem_trywait((sem_t *)sem_mem.addr) && SHUTDOWN_SIG != skid_sig_hand_signum)
        {
            results = errno;
            if (EAGAIN == results)
            {
                FPRINTF_ERR("%s SERVER - Still waiting on the semaphore.\n", DEBUG_INFO_STR);
                results = ENOERR;  // Reset and keep waiting
                sleep(1);  // Avoid the thrash
            }
            else
            {
                PRINT_ERROR(The call to sem_trywait() truly failed);
                PRINT_ERRNO(results);
                break;  // Something went truly wrong
            }
        }
        if (ENOERR == results)
        {
            if (strlen((char *)virt_mem.addr) > 0)
            {
                // Read it
                printf("MEM: %s\n", (char *)virt_mem.addr);
                // Clear it
                ((char *)virt_mem.addr)[0] = '\0';  // Truncate the buffer
            }
            // Release the lock
            results = release_semaphore((sem_t *)sem_mem.addr);
        }
    }
    // SHUTDOWN_SIG?
    if (SHUTDOWN_SIG == skid_sig_hand_signum)
    {
        fprintf(stdout, "\n%s is exiting\n", argv[0]);
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
    // Named semaphore pointer
    if (NULL != sem_ptr)
    {
        close_named_sem(&sem_ptr);  // Best effort
    }
    // Named semaphore special file
    if (NULL != sem_name)
    {
        remove_named_sem(sem_name);  // Best effort
    }

    // DONE
    return results;
}


int create_new_shm_obj(const char *name, size_t size, int *errnum)
{
    // LOCAL VARIABLES
    int result = validate_skid_err(errnum);  // Errno values
    int flags = O_CREAT | O_RDWR;            // shm_open() flags
    int shmfd = SKID_BAD_FD;                 // Shared memory object file descriptor
    bool truncate = true;                    // Size the new shared memory object
    // Permission bits for the shared memory object
    mode_t mode = SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | \
                  SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | \
                  SKID_MODE_OTHER_R | SKID_MODE_OTHER_W;

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


void print_shutdown(const char *prog_name, const char *shm_path, const char *sem_path)
{
    fprintf(stdout, "%s has begun waiting for entries in a shared memory object: %s\n",
            prog_name, shm_path);
    fprintf(stdout, "This IPC resource is managed by a named semaphore: %s\n", sem_path);
    fprintf(stdout, "Terminate the server by sending signal [%d] %s\n",
            SHUTDOWN_SIG, strsignal(SHUTDOWN_SIG));
    fprintf(stdout, "E.g., kill -%d %jd\n", SHUTDOWN_SIG, (intmax_t)getpid());
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
    int result = validate_skid_shared_name(name);  // Errno values

    // DONE
    return result;
}
