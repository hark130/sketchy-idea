/*
 *  Source file to highlight key differences between source compiled in glibc and musl.
 *  The portable source code must be computationally expensive to help highlight relative speeds.
 *  This binary will implement the Sieve of Eratosthenes for a given range of inclusive values.
 *  Values will be converted to unsigned long long ints and then processed.
 *  This source file purposely avoids utilizing skid_* libraries to make compilation easier.
 *
 *  Copy/paste the following...

./code/dist/test_misc_glibc_vs_musl.bin <RANGE_BEGIN> <RANGE_END>

 *
 */

#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */
#define SIEVE_START_PRIME ((unsigned long long int)2)

#include <errno.h>                  // EINVAL, ERANGE
#include <stdio.h>                  // fprintf()
#include <stdlib.h>                 // exit(), strtoull()
#include <string.h>                 // strlen()

/*
 *  Convert num into a numerical value stored in the out parameter "value".
 *  Returns errno values on failure: EINVAL for bad input, ERANGE for failed conversion.
 */
int convert_args(const char *num, unsigned long long int *value);

/*
 *  Convert a string to an unsigned long long int using strtoull().
 *  Returns 0 on failure (check errnum: EINVAL for bad input, ERANGE for failed conversion).
 */
unsigned long long int convert_str_to_pos_ull(const char *string, int *errnum);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *  Run the Sieve of Eratosthenes on an inclusive range of values beginning with 2 and ending
 *  with end.  Results are stored in a heap-allocated, zero-terminated array.  The caller is
 *  responsible for free()ing the array.
 *  Returns pointer on success, NULL on failure (see: errnum for details).
 */
unsigned long long int* sieve_it(unsigned long long int end, int *errnum);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;                  // Errno values from execution
    unsigned long long int end = 0;          // End of the range
    unsigned long long int* primes = NULL;   // Zero-terminated array of primes between 2 and end
    unsigned long long int* tmp_ptr = NULL;  // Temp-pointer into the primes array

    // INPUT VALIDATION
    if (argc != 2)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }
    else
    {
        exit_code = convert_args(argv[1], &end);
    }

    // SIEVE IT
    if (ENOERR == exit_code)
    {
        printf("End: %llu\n", end);  // DEBUGGING
        primes = sieve_it(end, &exit_code);
    }

    // PRINT IT
    if (ENOERR == exit_code)
    {
        tmp_ptr = primes;
        while (NULL != tmp_ptr && 0x0 != *tmp_ptr)
        {
            printf("Prime: %llu\n", *tmp_ptr);
            tmp_ptr++;
        }
    }

    // DONE
    exit(exit_code);
}


int convert_args(const char *num, unsigned long long int *value)
{
    // LOCAL VARAIBLES
    int results = ENOERR;                // Errno values
    unsigned long long int num_val = 0;  // Value of num

    // INPUT VALIDATION
    if (NULL == num || NULL == value)
    {
        results = EINVAL;
    }

    // CONVERT IT
    if (ENOERR == results)
    {
        num_val = convert_str_to_pos_ull(num, &results);
    }

    // STORE THEM
    if (ENOERR == results)
    {
        *value = num_val;
    }

    // DONE
    return results;
}


unsigned long long int convert_str_to_pos_ull(const char *string, int *errnum)
{
    // LOCAL VARIABLES
    unsigned long long int value = 0;  // Converted value
    int results = ENOERR;              // Errno values
    char *end_ptr = NULL;              // stroull() endptr

    // INPUT VALIDATION
    if (NULL == errnum || NULL == string || 0x0 == *string || '-' == *string)
    {
        results = EINVAL;
    }

    // CONVERT IT
    if (ENOERR == results)
    {
        errno = ENOERR;  // Manual reset
        value = strtoull(string, &end_ptr, 10);  // Convert string using base 10
        results = errno;
        if (ENOERR != results)
        {
            fprintf(stderr, "The call to stroull(%s) resulted in errno [%d]: %s\n",
                    string, results, strerror(results));
        }
        if (string == end_ptr)
        {
            fprintf(stderr, "The call to stroull(%s) indicated no digits were found\n",
                    string);
            if (ENOERR == results)
            {
                results = ERANGE;  // Failed conversion
            }
        }
    }

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return value;
}


void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <RANGE_END>\n", prog_name);
}


unsigned long long int* sieve_it(unsigned long long int end, int *errnum)
{
    // LOCAL VARIABLES
    int results = ENOERR;                   // Errno values
    unsigned long long int* primes = NULL;  // Heap-allocated array to store prime values

    // INPUT VALIDATION
    if (NULL == errnum)
    {
        results = EINVAL;
    }
    else if (SIEVE_START_PRIME >= end)
    {
        results = EINVAL;
    }

    // SIEVE IT
    /* TO DO: DON'T DO NOW... */

    // DONE
    if (NULL != errnum)
    {
        *errnum = results;
    }
    return primes;
}
