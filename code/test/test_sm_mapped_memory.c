/*
 *  Manually test skid_memory's map_skid_mem() and unmap_skid_mem() functions.
 *
 *  This manual test code performs the following actions:
 *  1. Validate input
 *  2. Fork
 *  3.a. Parent:
 *      i.   Maps memory to store a file's content
 *      ii.  Reads the file into the mapped memory
 *      iii. Release that resource
 *  3.b. Child:
 *      i.   Wait for the resource
 *      ii.  Read and print the resource
 *      iii. Unmap the mapped memory
 *
 *  Copy/paste the following...

./code/dist/test_sm_mapped_memory.bin <REGULAR_FILENAME>

 *
 */

// Standard includes
#include <errno.h>                          // EINVAL
#include <semaphore.h>                      // sem_init(), sem_post(), sem_trywait()
#include <stdbool.h>                        // false
#include <stdint.h>                         // SIZE_MAX
#include <stdio.h>                          // fprintf()
// #include <stdlib.h>                 // exit()
#include <sys/wait.h>                       // waitpid()
#include <unistd.h>                         // fork()
// Local includes
#define SKID_DEBUG                          // The DEBUG output is doing double duty as test output
// #include "devops_code.h"            // call_sigqueue()
#include "skid_debug.h"                     // PRINT_ERROR()
// #include "skid_macros.h"            // ENOERR
#include "skid_memory.h"                    // map_skid_mem(), skidMemMapRegion, unmap_skid_mem()
#include "skid_file_metadata_read.h"        // is_regular_file()
#include "skid_file_operations.h"           // read_file()
// #include "skid_signal_handlers.h"   // handle_ext_read_queue_int()
// #include "skid_signals.h"           // set_signal_handler_ext(), translate_signal_code()

/*
 *  Safely(?) convert a signed off_t value to a size_t value.
 */
size_t convert_offset(off_t offset, int *errnum);


/*
 *  Size filename, map a memory region into map_file, and then read filename into it.
 *  Does not validate input.  Returns ENOERR on success, errno value of failure.
 */
int run_parent(const char *filename, skidMemMapRegion_ptr map_file);


/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int results = ENOERR;               // Store errno and/or results here
    int prot = PROT_READ | PROT_WRITE;  // mmap() protections
    int flags = MAP_SHARED;             // mmap() flags
    pid_t my_pid = 0;                   // My PID
    pid_t wait_ret = 0;                 // Return value from the call to waitpid()
    int child_status = 0;               // Status information about the child process
    // sem_t *sem = NULL;               // Semaphore to manage the virtual memory region
    int pshared = 1;                    // Sempahore pshared value: 1 means process-shared
    unsigned int sem_val = 0;           // Initial value of the semaphore: locked by the parent
    skidMemMapRegion map_sem;           // Sempahore to manage the virtual memory region
    skidMemMapRegion map_file;          // argv[1] mapped into shared memory

    // INPUT VALIDATION
    if (argc != 2)
    {
        print_usage(argv[0]);
        results = EINVAL;
    }
    else if (false == is_regular_file(argv[1], &results))
    {
        fprintf(stderr, "%s much be a regular file.\n", argv[1]);
        print_usage(argv[0]);
        if (ENOERR == results)
        {
            results = EINVAL;
        }
    }

    // SETUP
    // Map a semaphore
    if (ENOERR == results)
    {
        map_sem.addr = NULL;
        map_sem.length = sizeof(sem_t);
        results = map_skid_mem(&map_sem, prot, flags);
    }
    // Initialize the semaphore
    if (ENOERR == results)
    {
        errno = ENOERR;  // Just in case
        if (0 != sem_init((sem_t *)(map_sem.addr), pshared, sem_val))
        {
            results = errno;
            if (ENOERR == results)
            {
                results = EINVAL;
            }
            PRINT_ERROR(The call to sem_init() failed);
            PRINT_ERRNO(results);
        }
    }

    // DO IT
    // Fork
    if (ENOERR == results)
    {
        my_pid = fork();
        if (-1 == my_pid)
        {
            results = errno;
            if (ENOERR == results)
            {
                results = EINTR;  // Just in case
            }
            PRINT_ERROR(The call to fork() failed);
            PRINT_ERRNO(results);
        }
    }
    // Parent
    if (ENOERR == results)
    {
        if (my_pid > 0)
        {
            results = run_parent(argv[1], &map_file);
            if (ENOERR == results)
            {
                errno = ENOERR;  // Just in case
                // Release the semaphore
                if (0 != sem_post((sem_t *)(map_sem.addr)))
                {
                    results = errno;
                    PRINT_ERROR(PARENT - The call to sem_post() failed);
                    PRINT_ERRNO(results);
                }
                else
                {
                    PRINT_ERROR(ABOUT TO RELEASE);  // DEBUGGING
                    FPRINTF_ERR("%s\n", (char *)map_file.addr);  // DEBUGGING
                    FPRINTF_ERR("%s PARENT - Semaphore released\n", DEBUG_INFO_STR);
                }
            }
            else
            {
                PRINT_ERROR(PARENT - The call to run_parent() failed);
                PRINT_ERRNO(results);
            }
            // Wait for the child
            while (ENOERR == results)
            {
                // Check the child process
                wait_ret = waitpid(my_pid, &child_status, WNOHANG);  // Is the child alive?
                if (my_pid == wait_ret)
                {
                    // The child's state has changed
                    if (WIFEXITED(child_status) || WIFSIGNALED(child_status) \
                        || WIFSTOPPED(child_status))
                    {
                        printf("%s PARENT - The child has exited\n",
                               DEBUG_INFO_STR);
                        break;
                    }
                }
                else if (0 == wait_ret)
                {
                    // The child didn't change state yet... be patient
                    // FPRINTF_ERR("%s The child did not change state yet\n", DEBUG_INFO_STR);
                }
                else if (-1 == wait_ret)
                {
                    // FPRINTF_ERR("%s Wait for any child process\n", DEBUG_INFO_STR);
                }
                else
                {
                    PRINT_ERROR(PARENT - The call to waitpid() reported an unknown PID);
                    FPRINTF_ERR("PARENT - The call to waitpid() reported an unknown PID: %d\n",
                                wait_ret);
                    break;
                }
            }
            // Clean up
            // Destroy the semaphore
            sem_destroy((sem_t *)map_sem.addr);  // Best effort
            // Unmap the semaphore memory
            PRINT_ERROR(HERE);  // DEBUGGING
            unmap_skid_mem(&map_sem);  // Best effort
        }
    }
    // Child
    if (ENOERR == results)
    {
        if (0 == my_pid)
        {
            // Obtain the lock
            while (-1 == sem_trywait((sem_t *)map_sem.addr))
            {
                results = errno;
                if (EAGAIN == results)
                {
                    FPRINTF_ERR("%s CHILD - Still waiting on the semaphore.\n", DEBUG_INFO_STR);
                    results = ENOERR;  // Reset and keep waiting
                    sleep(1);  // Avoid the thrash
                }
                else
                {
                    break;  // Something went truly wrong
                }
                PRINT_ERROR(RESULTS);  // DEBUGGING
                FPRINTF_ERR("RESULTS %d\n", results);  // DEBUGGING
            }
            PRINT_ERROR(RESULTS);  // DEBUGGING
            FPRINTF_ERR("RESULTS %d\n", results);  // DEBUGGING
            // Print the resource
            if (ENOERR == results)
            {
                PRINT_ERROR(ABOUT TO PRINT);  // DEBUGGING
                FPRINTF_ERR("%s\n", (char *)map_file.addr);  // DEBUGGING
                for (size_t i = 0; i < map_file.length; i++)
                {
                    if (EOF == putchar((*(((char *)map_file.addr) + i))))
                    {
                        results = EIO;  // As good an errno value as any
                        PRINT_ERROR(CHILD - The call to putchar() failed);
                        break;
                    }
                }
                putchar('\n');
            }
            // Clean up
            unmap_skid_mem(&map_file);  // Best effort
        }
    }

    // DONE
    return results;
}


size_t convert_offset(off_t offset, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Errno values
    size_t new_val = 0;   // offset converted to a size_t

    // INPUT VALIDATION
    if (NULL == errnum)
    {
        result = EINVAL;
    }
    else if (offset < 0 || SIZE_MAX < offset)
    {
        result = ERANGE;
    }
    else
    {
        new_val = (size_t)offset;
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return new_val;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <REGULAR_FILENAME>\n", prog_name);
}


int run_parent(const char *filename, skidMemMapRegion_ptr map_file)
{
    // LOCAL VARIABLES
    int result = ENOERR;                // Errno values
    off_t file_size = 0;                // Size of filename
    size_t conv_file_size = 0;          // file_size converted to size_t
    int prot = PROT_READ | PROT_WRITE;  // mmap() protections
    int flags = MAP_SHARED;             // mmap() flags
    char *file_cont = NULL;             // Temporary heap pointer with file contents

    // RUN IT
    // Size filename
    file_size = get_size(filename, &result);
    if (ENOERR == result)
    {
        conv_file_size = convert_offset(file_size, &result);
    }
    // Map a memory region
    if (ENOERR == result)
    {
        map_file->addr = NULL;
        map_file->length = conv_file_size;
        result = map_skid_mem(map_file, prot, flags);
        if (ENOERR != result)
        {
            PRINT_ERROR(The call to map_skid_mem() failed);
            PRINT_ERRNO(result);
        }
    }
    // Read filename into it
    if (ENOERR == result)
    {
        file_cont = read_file(filename, &result);
        if (ENOERR == result)
        {
            memcpy(map_file->addr, file_cont, conv_file_size);
        }
    }

    // CLEANUP
    if (NULL != file_cont)
    {
        free_skid_string(&file_cont);  // Best effort
    }

    // DONE
    return result;
}
