/*
 *  Manually test skid_assembly's inline assembly functionality.
 *  This binary makes use of the read_cpu_tsc() to read the processor's timestamp counter.
 *
 *  Copy/paste the following...

./code/dist/test_sa_read_cpu_tsc.bin

 *
 */

#ifndef SKID_DEBUG
#define SKID_DEBUG                  // Enable DEBUG logging
#endif  /* SKID_DEBUG */
#define WAIT_SLEEP 1                // Number of seconds to wait
#define NUM_LOOPS 10                // Number of times to loop

#include <errno.h>                  // EINVAL
#include <inttypes.h>               // PRIu64
#include <stdio.h>                  // printf()
#include <stdlib.h>                 // exit()
#include <unistd.h>                 // sleep()
#include "skid_assembly.h"          // read_cpu_tsc()
#include "skid_macros.h"            // ENOERR

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;  // Errno values
    uint64_t timestamp = 0;  // Processor's timestamp counter value

    // INPUT VALIDATION
    if (argc != 1)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }

    // GET IT
    if (ENOERR == exit_code)
    {
        for (int i = 0; i < NUM_LOOPS; i++)
        {
            timestamp = read_cpu_tsc();
            printf("The timestamp is currently: %" PRIu64 "\n", timestamp);
            sleep(WAIT_SLEEP);
        }
    }

    // DONE
    exit(exit_code);
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s\n", prog_name);
}
