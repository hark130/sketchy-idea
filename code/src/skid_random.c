/*
 *  This library defines functionality to randomize values.
 */

// #define SKID_DEBUG                          // Enable DEBUG logging

#include <errno.h>                          // EINVAL
#include <stdbool.h>                        // bool, false, true
#include <stdlib.h>                         // rand()
#include <unistd.h>                         // getpid()
#include "skid_debug.h"                     // PRINT_ERROR()
#include "skid_macros.h"                    // ENOERR, SKID_INTERNAL
#include "skid_random.h"                    // public functions
#include "skid_time.h"                      // get_unix_time()
#include "skid_validation.h"                // validate_skid_err()

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *  Description:
 *      Use the number of seconds since the Epoch as the seed for a new sequence of pseudo-random
 *      integers to be returned by rand().
 *
 *  Returns:
 *      ENOERR on success, errno value on failure.
 */
SKID_INTERNAL int seed_it(void);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


unsigned int randomize_number(unsigned int stop, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;         // Store errno value
    unsigned int random_num = 0;  // Randomized number between 1 and stop, inclusive

    // INPUT VALIDATION
    results = validate_skid_err(errnum);

    // RANDOMIZE IT
    if (ENOERR == results)
    {
        random_num = randomize_range(1, stop, &results);
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return random_num;
}


unsigned int randomize_range(unsigned int start, unsigned int stop, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;         // Store errno value
    unsigned int random_num = 0;  // Randomized number between 1 and stop, inclusive

    // INPUT VALIDATION
    results = validate_skid_err(errnum);
    if (ENOERR == results)
    {
        if (start >= stop)
        {
            results = ERANGE;
        }
    }

    // RANDOMIZE IT
    // Seed it
    if (ENOERR == results)
    {
        results = seed_it();
    }
    // Get it
    if (ENOERR == results)
    {
        random_num = (rand() % (stop - start + 1)) + start;
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return random_num;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


SKID_INTERNAL int seed_it(void)
{
    // LOCAL VARIABLES
    int results = ENOERR;              // Store errno value
    time_t num_sec = SKID_BAD_TIME_T;  // Number of seconds since the Epoch
    static bool seeded = false;        // Only seed once per process

    // SEED IT
    if (false == seeded)
    {
        num_sec = get_unix_time(&results);
        if (ENOERR == results)
        {
            srand(num_sec ^ getpid());  // Unique(?) seed per PID
            seeded = true;  // Only seed it once
        }
    }

    // DONE
    return results;
}
