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

#include <errno.h>                  // EINVAL, ERANGE
#include <stdio.h>                  // fprintf()
#include <stdlib.h>                 // exit(), strtoull()
#include <string.h>                 // strlen()

/*
 *  Convert num1 and num2 into numerical values stored in the out parameters: low, high.
 *  The value in low will be less than the value in high.
 *  Returns errno values on failure: EINVAL for bad input, ERANGE for failed conversion.
 */
int convert_args(const char *num1, const char *num2, unsigned long long int *low,
                 unsigned long long int *high);

/*
 *  Convert a string to an unsigned long long int using strtoull().
 *  Returns 0 on failure (check errnum: EINVAL for bad input, ERANGE for failed conversion).
 */
unsigned long long int convert_str_to_pos_ull(const char *string, int *errnum);

/*
 *  Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int exit_code = ENOERR;           // Errno values from execution
    unsigned long long int low = 0;   // Start of the range
    unsigned long long int high = 0;  // End of the range

    // INPUT VALIDATION
    if (argc != 3)
    {
       print_usage(argv[0]);
       exit_code = EINVAL;
    }
    else
    {
        exit_code = convert_args(argv[1], argv[2], &low, &high);
    }

    // SIEVE IT
    if (ENOERR == exit_code)
    {
        printf("Low: %llu\nHigh: %llu\n", low, high);  // DEBUGGING
    }

    // DONE
    exit(exit_code);
}


int convert_args(const char *num1, const char *num2, unsigned long long int *low,
                 unsigned long long int *high)
{
    // LOCAL VARAIBLES
    int results = ENOERR;             // Errno values
    unsigned long long int val1 = 0;  // Value of num1
    unsigned long long int val2 = 0;  // Value of num2

    // INPUT VALIDATION
    // The num1 and num2 args will be validated by convert_str_to_pos_ull()
    if (NULL == low || NULL == high)
    {
        results = EINVAL;
    }

    // CONVERT IT
    // num1
    if (ENOERR == results)
    {
        val1 = convert_str_to_pos_ull(num1, &results);
    }
    // num2
    if (ENOERR == results)
    {
        val2 = convert_str_to_pos_ull(num2, &results);
    }
    // Validate order
    if (ENOERR == results)
    {
        if (val1 == val2)
        {
            fprintf(stderr, "The two values may not be equal\n");
            results = EINVAL;
        }
        else if (val1 > val2)
        {
            val1 = val1 ^ val2;
            val2 = val1 ^ val2;
            val1 = val1 ^ val2;
        }
    }

    // STORE THEM
    if (ENOERR == results)
    {
        *low = val1;
        *high = val2;
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
    fprintf(stderr, "Usage: %s <RANGE_BEGIN> <RANGE_END>\n", prog_name);
}
