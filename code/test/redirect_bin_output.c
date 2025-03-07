/*
 *  This source file utilizes SKETCHY IDEA (SKID) to implement redirect_bin_output.bin.
 */

#include "skid_file_descriptors.h"  // close_fd(), open_fd()
#include "skid_macros.h"            // ENOERR, SKID_BAD_FD
#include "skid_time.h"              // build_timestamp()

/*
 *  Build a unique timestamped filename which follows this format:
 *      <TIMESTAMP>-XXXXXX-<PROPER NAME>-<TYPE>.txt
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

    // INPUT VALIDATION
    if (argc < 2)
    {
        exit_code = EINVAL;  // There was no binary to redirect output for
        print_usage(argv[0]);
    }

    // REDIRECT IT
    // 1. Build Filenames
    // Stdout
    if (ENOERR == exit_code)
    {
        stdout_fn = build_filename(argv[1], "output", &exit_code);
    }
    // Stderr
    if (ENOERR == exit_code)
    {
        stderr_fn = build_filename(argv[1], "errors", &exit_code);
    }
    // 2. Open Filenames
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
    // 3. Fork

    // CLEANUP
    // Stdout filename
    if (NULL != stdout_fn)
    {
        free_skid_mem(&stdout_fn);
    }
    // Stderr filename
    if (NULL != stderr_fn)
    {
        free_skid_mem(&stderr_fn);
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
    int results = ENOERR;   // Results of execution
    char *built_fn = NULL;  // Built filename

    // INPUT VALIDATION
    if (NULL == timestamp || NULL == proper_name || NULL == type || NULL == errnum)
    {
        results = EINVAL;  // NULL pointer
    }

    // BUILD IT
    // Get temp filename
    if (ENOERR != results)
    {
        
    }
    // Finish filename
    if (ENOERR != results)
    {
        
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
