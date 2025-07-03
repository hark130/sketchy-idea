/*
 *  Manually test skid_memory.h's mapped memory functionality and
 *  skid_signal_handlers.h's handle_ext_read_queue_ptr() functions.
 *
 *  This manual test code performs the following actions:
 *  1. Install the handle_ext_read_queue_ptr() extended signal handler for SIGUSR2
 *  2. Map shared memory to be used between parent and child
 *  3. Fork
 *  3.a. Parent
 *      3.a.i.      Read filename into mapped memory
 *      3.a.ii.     Send a signal to the child process with the mapped memory address
 *      3.a.iii.    Wait for the child to finish
 *  3.b. Child
 *      3.b.i.      Wait for a signal from the parent
 *      3.b.ii.     Retrieve the memory address from the signal handler
 *      3.b.iii.    Read the contents of that memory address
 *
 *  Copy/paste the following...

./code/dist/test_sm_handle_ext_read_queue_ptr.bin <REGULAR_FILENAME>

 *
 */

// Standard includes
// #include <errno.h>                  // EINVAL, EINTR, EIO
#include <stdbool.h>                  // bool, false, true
#include <stdint.h>                   // SIZE_MAX
// #include <stdio.h>                  // fprintf()
#include <stdlib.h>                   // exit()
#include <sys/wait.h>                 // waitpid()
#include <unistd.h>                   // fork()
// Local includes
#define SKID_DEBUG                    // The DEBUG output is doing double duty as test output
#include "skid_debug.h"               // DEBUG_* macros, PRINT_*()
#include "skid_file_metadata_read.h"  // get_size()
#include "skid_file_operations.h"     // read_file()
#include "skid_macros.h"              // ENOERR
#include "skid_memory.h"              // free_skid_string()
#include "skid_signal_handlers.h"     // handle_ext_read_queue_ptr()
#include "skid_signals.h"             // block_sig*(), set_sig*_handler_ext(), translate_sig*_code()

/*
 *  Description:
 *      Queue a signal, with integer data, to a PID by calling sigqueue() (see: sigqueue(3)).
 *
 *  Args:
 *      pid: The PID to queue the signal for.
 *      signum: The signal number to queue.
 *      sival_ptr: The pointer value to include with the signal.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int call_sigqueue_ptr(pid_t pid, int signum, void *sival_ptr);

/*
 *  Safely(?) convert a signed off_t value to a size_t value.
 */
size_t convert_offset(off_t offset, int *errnum);

/*
 *  Get the file size of filename.
 */
size_t get_file_size(const char *filename, int *errnum);

/*
 *  Read the file into mapped memory.
 */
int map_filename(const char *filename, skidMemMapRegion_ptr file_map);

/*
 *  Size the file and map the memory.
 */
int prepare_mapping(const char *filename, skidMemMapRegion_ptr file_map);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Call this when the signal handler indicates there's a pointer to retrieve.
 *  1. Verifies atomic variables (e.g., set, right signal number)
 *  2. Reads and prints the string
 *  3. Resets atomic variables
 *  4. Unblocks the signal (to allow the handler to process the next message in the queue)
 *
 *  Returns ENOERR on success, errno on failure.
 */
int process_queue(int signum);

/*
 *  2. Reads and prints the string
 */
int process_queue_read(void);

/*
 *  3. Resets atomic variables
 */
void process_queue_reset_flags(void);

/*
 *  4. Unblocks the signal (to allow the handler to process the next message in the queue)
 */
int process_queue_unblock(int signum);

/*
 *  1. Verifies atomic variables (e.g., set, right signal number)
 */
int process_queue_verify(int signum);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = 0;          // Store errno and/or results here
    int flags = SA_RESTART;     // Modifies signal behavior: restart system calls across signals
    int signum = SIGUSR2;       // Signal number to use for communication
    pid_t the_pid = 0;          // Return value from fork()
    pid_t wait_ret = 0;         // Return value from the call to waitpid()
    int child_status = 0;       // Status information about the child process
    char *filename = NULL;      // argv[1] <REGULAR_FILENAME>
    skidMemMapRegion file_map;  // Mapped contents of the filename

    // INPUT VALIDATION
    if (argc != 2)
    {
        print_usage(argv[0]);
        exit_code = EINVAL;
    }
    else if (false == is_regular_file(argv[1], &exit_code))
    {
        fprintf(stderr, "%s much be a regular file.\n", argv[1]);
        print_usage(argv[0]);
        if (ENOERR == exit_code)
        {
            exit_code = EINVAL;
        }
    }
    else
    {
        filename = argv[1];
    }

    // RUN IT
    // Block a signal
    if (ENOERR == exit_code)
    {
        fprintf(stdout, "%s Blocking signal %d: %s\n", DEBUG_INFO_STR, signum, strsignal(signum));
        exit_code = block_signal(signum, NULL);
        if (exit_code)
        {
            PRINT_ERROR(The call to block_signal() failed);
            PRINT_ERRNO(exit_code);
        }
        else
        {
            printf("%s %s now temporarily blocking signal %d.\n", DEBUG_INFO_STR, argv[0], signum);
        }
    }
    // Prepare mapped memory
    if (ENOERR == exit_code)
    {
        exit_code = prepare_mapping(filename, &file_map);
    }
    // 1. Setup Signal Handler
    if (ENOERR == exit_code)
    {
        exit_code = set_signal_handler_ext(signum, handle_ext_read_queue_ptr, flags, NULL);
        if (exit_code)
        {
            PRINT_ERROR(The call to set_signal_handler_ext() failed);
            PRINT_ERRNO(exit_code);
        }
        else
        {
            printf("%s %s is now handling signal %d: %s.\n", DEBUG_INFO_STR, argv[0],
                   signum, strsignal(signum));
        }
    }

    // 2. Fork
    if (ENOERR == exit_code)
    {
        the_pid = fork();
        if (-1 == the_pid)
        {
            exit_code = errno;
            if (ENOERR == exit_code)
            {
                exit_code = EINTR;  // Just in case
            }
            PRINT_ERROR(The call to fork() failed);
            PRINT_ERRNO(exit_code);
        }
    }

    // 3. Execute
    // Parent
    if (ENOERR == exit_code)
    {
        if (the_pid > 0)
        {
            // Read filename into mapped memory
            exit_code = map_filename(filename, &file_map);
            if (ENOERR == exit_code)
            {
                // Send a signal to the child process with the mapped memory address
                exit_code = call_sigqueue_ptr(the_pid, signum, file_map.addr);
                if (ENOERR != exit_code)
                {
                    PRINT_ERROR(PARENT - The call_sigqueue_ptr() function failed);
                    PRINT_ERRNO(exit_code);
                }
            }
            else
            {
                PRINT_ERROR(PARENT - The call to map_filename() failed);
                PRINT_ERRNO(exit_code);
            }
            // Wait for the child to finish
            while (ENOERR == exit_code)
            {
                // Check the child process
                wait_ret = waitpid(the_pid, &child_status, WNOHANG);  // Is the child alive?
                if (the_pid == wait_ret)
                {
                    // The child's state has changed
                    if (WIFEXITED(child_status) || WIFSIGNALED(child_status) \
                        || WIFSTOPPED(child_status))
                    {
                        // The child is no more
                        printf("%s PARENT - The child has finished\n",
                               DEBUG_INFO_STR);
                        break;
                    }
                }
                else if (0 == wait_ret)
                {
                    // The child didn't change state yet... be patient
                }
                else if (-1 == wait_ret)
                {
                    PRINT_ERROR(PARENT - The waitpid() call failed);
                    exit_code = child_status;
                    PRINT_ERRNO(exit_code);
                    break;
                }
                else
                {
                    PRINT_ERROR(PARENT - The call to waitpid() reported an unknown PID);
                    FPRINTF_ERR("PARENT - The call to waitpid() reported an unknown PID: %d\n",
                                wait_ret);
                    break;
                }
            }

            // CLEAN UP
            unmap_skid_mem(&file_map);  // Best effort
        }
    }

    // Child
    if (ENOERR == exit_code)
    {
        if (0 == the_pid)
        {
            exit_code = unblock_signal(signum, NULL);  // Receive data
            while (ENOERR == exit_code)
            {
                // Process Data
                exit_code = process_queue(signum);
                if (ENODATA == exit_code)
                {
                    exit_code = ENOERR;  // Ignore a lack of data
                }
                else
                {
                    break;  // Either we read it or there was an error
                }
            }
        }
    }

    // DONE
    exit(exit_code);
}


int call_sigqueue_ptr(pid_t pid, int signum, void *sival_ptr)
{
    // LOCAL VARIABLES
    int result = ENOERR;                     // Results of execution
    union sigval data = { .sival_ptr = 0 };  // Data to send via sigqueue()

    // PREPARE
    data.sival_ptr = sival_ptr;

    // QUEUE IT
    if (sigqueue(pid, signum, data))
    {
        result = errno;
        PRINT_ERROR(The call to sigqueue() failed);
        if (ENOERR == result)
        {
            result = EINTR;  // Use this merely to indicate an error occurred
        }
        else
        {
            PRINT_ERRNO(result);
        }
        FPRINTF_ERR("Attempted to sigqueue(%d, %d, %p)\n", pid, signum, sival_ptr);
    }

    // DONE
    return result;
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


size_t get_file_size(const char *filename, int *errnum)
{
    // LOCAL VARIABLES
    int result = ENOERR;        // Results of execution
    off_t file_size = 0;        // Filename's file size
    size_t conv_file_size = 0;  // Filename's file size (off_t) converted to size_t

    // INPUT VALIDATION

    // GET IT
    // Get the off_t file size
    if (ENOERR == result)
    {
        file_size = get_size(filename, &result);  // Size filename
    }
    // Convert off_t to size_t
    if (ENOERR == result)
    {
        conv_file_size = convert_offset(file_size, &result);  // Convert the value
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return conv_file_size;
}


int map_filename(const char *filename, skidMemMapRegion_ptr file_map)
{
    // LOCAL VARIABLES
    int result = ENOERR;     // Results of execution
    char *file_cont = NULL;  // Filename contents

    // INPUT VALIDATION
    if (NULL == filename || NULL == file_map)
    {
        result = EINVAL;  // NULL pointers
    }

    // MAP IT
    // Read the file
    if (ENOERR == result)
    {
        file_cont = read_file(filename, &result);
    }
    // Copy file contents into mapped memory
    if (ENOERR == result)
    {
        memcpy(file_map->addr, file_cont, file_map->length - 1);
    }

    // CLEAN UP
    if (NULL != file_cont)
    {
        free_skid_string(&file_cont);  // Best effort
    }

    // DONE
    return result;
}


int prepare_mapping(const char *filename, skidMemMapRegion_ptr file_map)
{
    // LOCAL VARIABLES
    int result = ENOERR;                // Results of execution
    size_t file_size = 0;               // Size of filename
    int prot = PROT_READ | PROT_WRITE;  // mmap() protections
    int flags = MAP_SHARED;             // mmap() flags

    // INPUT VALIDATION
    if (NULL == filename || NULL == file_map)
    {
        result = EINVAL;  // NULL pointers
    }

    // MAP IT
    // Size the file
    if (ENOERR == result)
    {
        file_size = get_file_size(filename, &result);
    }
    // Map memory
    if (ENOERR == result)
    {
        file_map->addr = NULL;
        file_map->length = file_size + 1;  // Nul-terminate this memory space
        result = map_skid_mem(file_map, prot, flags);
    }

    // DONE
    return result;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <REGULAR_FILENAME>\n", prog_name);
}


int process_queue(int signum)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Results of execution

    // PROCESS IT
    // 1. Verifies atomic variables (e.g., set, right signal number)
    result = process_queue_verify(signum);

    // 2. Reads and prints the string
    if (ENOERR == result)
    {
        result = process_queue_read();
    }

    // 3. Resets atomic variables
    if (ENOERR == result)
    {
        process_queue_reset_flags();
    }

    // 4. Unblocks the signal (to allow the handler to process the next message in the queue)
    if (ENOERR == result)
    {
        result = process_queue_unblock(signum);
    }

    // DONE
    return result;
}


int process_queue_read(void)
{
    // LOCAL VARIABLE
    int result = ENOERR;                          // Results of execution
    char *data = (char *)skid_sig_hand_data_ptr;  // Pointer from the queue
    int pid = skid_sig_hand_pid;                  // PID of the sending process
    int code = skid_sig_hand_sigcode;             // Signal code
    int signal = skid_sig_hand_signum;            // Signal number
    int uid = skid_sig_hand_uid;                  // Real UID of the sending process
    char *sig_code_str = NULL;                    // Human-readable description of the signal code

    // PROCESS IT
    // Translate the signal code
    if (ENOERR == result)
    {
        sig_code_str = translate_signal_code(signal, code, &result);
        if (result)
        {
            PRINT_ERROR(The call to translate_signal_code() failed);
            PRINT_ERRNO(result);
        }
    }
    // Print all the details
    if (ENOERR == result)
    {
        fprintf(stderr, "%s CHILD - Received value %p inside signal number %d (%s) code "
                "%d (%s) from UID %d at PID %d\n", DEBUG_INFO_STR, data, signal, strsignal(signal),
                code, sig_code_str, uid, pid);
        fprintf(stderr, "%s CHILD - The contents of the mapped memory:\n\n%s\n",
                DEBUG_INFO_STR, data);
    }

    // CLEANUP
    if (NULL != sig_code_str)
    {
        free_skid_string(&sig_code_str);  // Best effort
    }

    // DONE
    return result;
}


void process_queue_reset_flags(void)
{
    skid_sig_hand_queue = 0;
    skid_sig_hand_data_ptr = NULL;
    skid_sig_hand_pid = 0;
    skid_sig_hand_sigcode = 0;
    skid_sig_hand_signum = 0;
    skid_sig_hand_uid = 0;
}


int process_queue_unblock(int signum)
{
    return unblock_signal(signum, NULL);
}


int process_queue_verify(int signum)
{
    // LOCAL VARIABLES
    int result = ENOERR;            // Results of execution
    QueueData_t ptr_val = Pointer;  // Only accepting pointer values in the queue

    // VERIFY VARIABLES
    if (ptr_val != skid_sig_hand_queue)
    {
        result = ENODATA;  // Nothing that we care about
    }
    else if (signum != skid_sig_hand_signum)
    {
        result = ENODATA;  // Wrong signal number
    }

    // DONE
    return result;
}
