/*
 *  This source file utilizes SKETCHY IDEA (SKID) to implement redirect_bin_output.bin.
 *  Uses the following exit codes:
 *      0      Success
 *      22     Bad input (EINVAL)
 *      132    The child process received a signal (ERFKILL)
 *      errno  Errno values are used for all other failures including the child process' exit code
 */

#define SKID_DEBUG                  // Enable DEBUGGING
#include "skid_debug.h"             // PRINT_ERRNO, PRINT_ERROR
#include "skid_file_descriptors.h"  // call_dup2(), close_fd(), open_fd()
#include "skid_macros.h"            // ENOERR, SKID_BAD_FD
#include "skid_memory.h"            // alloc_skid_mem(), free_skid_mem()
#include "skid_time.h"              // build_timestamp()
#include "skid_validation.h"        // validate_skid_err()
#include <errno.h>                  // EINVAL
#include <fcntl.h>                  // open() flag macros
#include <libgen.h>                 // basename()
#include <stdlib.h>                 // exit()
#include <stdio.h>                  // fprintf(), sprintf()
#include <string.h>                 // strlen()
#include <sys/types.h>              // pid_t
#include <sys/wait.h>               // waitpid()
#include <unistd.h>                 // fork()


#define FILE_EXT ".txt"             // File extension to use for the output files


/*
 *  Build a unique timestamped filename which follows this format:
 *      <TIMESTAMP>-<PROPER NAME>-<TYPE>.txt
 *
 *  Args:
 *      timestamp: Non-empty datetime stamp (e.g., YYYYMMDD-HHMMSS).
 *      proper_name: Should likely be the name of the binary being executed-and-redirected.
 *      type: Expected to be "output" (for stdout) or "errors" (for stderr).
 *      errnum: [Out] Storage location for errno values encountered.
 */
char *build_filename(const char *timestamp, const char *proper_name, const char *type, int *errnum);

/*
 *  Description:
 *      Print the usage.
 *
 *  Args:
 *      prog_name: argv[0].
 */
void print_usage(const char *prog_name);


/*
 *  Description:
 *      Replace characters that aren't filename-friendly with underscores.
 *
 *  Args:
 *      filename: A filename to modify.
 *
 *  Returns:
 *      0 on success, errno on error.
 */
int sanitize_filename(char *filename);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = 0;                                              // Store errno here
    char *stdout_fn = NULL;                                         // Stdout filename
    char *stderr_fn = NULL;                                         // Stderr filename
    int stdout_fd = 0;                                              // Stdout file descriptor
    int stderr_fd = 0;                                              // Stderr file descriptor
    int flags = O_WRONLY | O_CREAT | O_CLOEXEC;                     // Flags for open()
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;  // Mode for open()
    char *timestamp = NULL;                                         // YYYYMMDD-HHMMSS
    char *bin_name = NULL;                                          // Sanitized copy of argv[1]
    pid_t pid = 0;                                                  // Return value from fork()
    pid_t retval = 0;                                               // Return value from waitpid()
    int wstatus = 0;                                                // Status of the child PID

    // INPUT VALIDATION
    if (argc < 2)
    {
        exit_code = EINVAL;  // There was no binary to redirect output for
        print_usage(argv[0]);
    }

    // REDIRECT IT
    // 1. Sanitize the binary name
    // Copy it
    if (ENOERR == exit_code)
    {
        bin_name = copy_skid_string(basename(argv[1]), &exit_code);
    }
    // Sanitize it
    if (ENOERR == exit_code)
    {
        exit_code = sanitize_filename(bin_name);
        if (ENOERR != exit_code)
        {
            PRINT_ERROR(The call to sanitize_filename() failed);
            PRINT_ERRNO(exit_code);
        }
    }
    // 2. Get Timestamp
    if (ENOERR == exit_code)
    {
        timestamp = build_timestamp(&exit_code);
    }
    // 3. Build Filenames
    // Stdout
    if (ENOERR == exit_code)
    {
        stdout_fn = build_filename(timestamp, bin_name, "output", &exit_code);
    }
    // Stderr
    if (ENOERR == exit_code)
    {
        stderr_fn = build_filename(timestamp, bin_name, "errors", &exit_code);
    }
    // 4. Open Filenames
    // Stdout
    if (ENOERR == exit_code)
    {
        stdout_fd = open_fd(stdout_fn, flags, mode, &exit_code);
    }
    // Stderr
    if (ENOERR == exit_code)
    {
        stderr_fd = open_fd(stderr_fn, flags, mode, &exit_code);
    }
    // 5. Fork
    if (ENOERR == exit_code)
    {
        pid = fork();
        // Error
        if (pid < 0)
        {
            exit_code = errno;
            PRINT_ERROR(The call to fork() failed);
            PRINT_ERRNO(exit_code);
        }
        // Child
        else if (0 == pid)
        {
            // Stdout
            if (SKID_STDOUT_FD != call_dup2(stdout_fd, SKID_STDOUT_FD, &exit_code))
            {
                PRINT_ERROR(The call to call_dup2() stdout failed);
                PRINT_ERRNO(exit_code);
            }
            else if (SKID_STDERR_FD != call_dup2(stderr_fd, SKID_STDERR_FD, &exit_code))
            {
                PRINT_ERROR(The call to call_dup2() stderr failed);
                PRINT_ERRNO(exit_code);
            }
            else
            {
                if (-1 == execvp(argv[1], argv + 1))
                {
                    exit_code = errno;
                    PRINT_ERROR(The call to execvp() failed);
                    PRINT_ERRNO(exit_code);
                }
            }
        }
        // Parent
        else
        {
            // Wait for the child to finish
            while (ENOERR == exit_code)
            {
                sleep(1);  // A tasteful sleep
                retval = waitpid(pid, &wstatus, WNOHANG);
                if (-1 == retval)
                {
                    exit_code = errno;
                    if (ENOERR == exit_code)
                    {
                        exit_code = EINVAL;  // Best guess
                    }
                    PRINT_ERROR(The call to waitpid() failed);
                    PRINT_ERRNO(exit_code);
                }
                else if (0 == retval)
                {
                    FPRINTF_ERR("%s Still waiting on the child's status to change\n",
                                DEBUG_INFO_STR);
                }
                else if (pid == retval)
                {
                    if (WIFEXITED(wstatus))
                    {
                        exit_code = WEXITSTATUS(wstatus);
                        FPRINTF_ERR("%s The child exited with %d\n",
                                    DEBUG_INFO_STR, exit_code);
                        break;  // The child exited!
                    }
                    else if (WIFSIGNALED(wstatus))
                    {
                        exit_code = ERFKILL;
                        FPRINTF_ERR("%s The child was kill by signal [%d] %s\n",
                                    DEBUG_INFO_STR, WTERMSIG(wstatus),
                                    strsignal(WTERMSIG(wstatus)));  // DEBUGGING
                    }
                    else if (WIFSTOPPED(wstatus))
                    {
                        exit_code = ERFKILL;
                        FPRINTF_ERR("%s The child was kill by signal [%d] %s\n",
                                    DEBUG_INFO_STR, WSTOPSIG(wstatus),
                                    strsignal(WSTOPSIG(wstatus)));  // DEBUGGING
                    }
                }
            };
        }
    }

    // CLEANUP
    // Timestamp
    if (NULL != timestamp)
    {
        free_skid_mem((void **)&timestamp);
    }
    // bin_name
    if (NULL != bin_name)
    {
        free_skid_mem((void **)&bin_name);
    }
    // Stdout filename
    if (NULL != stdout_fn)
    {
        free_skid_mem((void **)&stdout_fn);
    }
    // Stderr filename
    if (NULL != stderr_fn)
    {
        free_skid_mem((void **)&stderr_fn);
    }
    // Stdout file descriptor
    if (SKID_BAD_FD != stdout_fd)
    {
        close_fd(&stdout_fd, true);  // Silence all logging/debugging
    }
    // Stderr file descriptor
    if (SKID_BAD_FD != stderr_fd)
    {
        close_fd(&stderr_fd, true);  // Silence all logging/debugging
    }

    // DONE
    exit(exit_code);
}


char *build_filename(const char *timestamp, const char *proper_name, const char *type, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;            // Results of execution
    size_t built_fn_len = 0;         // The calculated length of the built filename
    char *built_fn = NULL;           // Built filename

    // INPUT VALIDATION
    if (NULL == timestamp || NULL == proper_name || NULL == type || NULL == errnum)
    {
        results = EINVAL;  // NULL pointer
    }
    else if (0x0 == *timestamp)
    {
        results = EINVAL;  // Empty string
    }

    // BUILD IT
    // Allocate a buffer
    if (ENOERR == results)
    {
        // <TIMESTAMP>-<PROPER NAME>-<TYPE>.txt
        built_fn_len = strlen(timestamp) + 1 + strlen(proper_name) + 1 + strlen(type) \
                       + strlen(FILE_EXT);
        built_fn = alloc_skid_mem(built_fn_len + 1, sizeof(char), &results);
    }
    // Finish filename
    if (ENOERR == results)
    {
        sprintf(built_fn, "%s-%s-%s%s", timestamp, proper_name, type, FILE_EXT);
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return built_fn;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <BINARY> [COMMAND LINE ARGUMENTS...]\n", prog_name);
}


int sanitize_filename(char *filename)
{
    // LOCAL VARIABLES
    int results = ENOERR;                   // Results of execution
    size_t fn_len = 0;                      // The length of filename
    char replacement = '_';                 // Replace unacceptable chars with this
    char bad_chars[] = { '.', '/', '\0' };  // Unacceptable characters
    char *bad_char = bad_chars;             // Iterating variable

    // INPUT VALIDATION
    if (NULL == filename)
    {
        results = EINVAL;  // NULL pointer
    }
    else
    {
        fn_len = strlen(filename);
    }

    // SANITIZE IT
    if (ENOERR == results)
    {
        for (int i = 0; i < fn_len; i++)
        {
            bad_char = bad_chars;  // Reset the iterating variable
            while (NULL != bad_char && 0x0 != *bad_char)
            {
                if (*bad_char == (*(filename + i)))
                {
                    (*(filename + i)) = replacement;
                    break;  // Found one so stop looking for more
                }
                else
                {
                    bad_char++;
                }
            }
        }
    }

    // DONE
    return results;
}
