/*
 *  Source file to demonstrate setjmp()/longjmp() usage.
 *
 *  Copy/paste these compiler commands to determine the results of the feature test macros:
 *

gcc -I ./code/include/ -o ./code/dist/test_misc_setjmp_longjmp_clean.bin ./code/test/test_misc_setjmp_longjmp.c && ./code/dist/test_misc_setjmp_longjmp_clean.bin; echo $?
gcc -I ./code/include/ -o ./code/dist/test_misc_setjmp_longjmp_obfus.bin ./code/test/test_misc_setjmp_longjmp.c -DUSE_OBFUSCATED_FLOW && ./code/dist/test_misc_setjmp_longjmp_obfus.bin; echo $?

 *  
 */


// #define USE_OBFUSCATED_FLOW

#ifdef USE_OBFUSCATED_FLOW
#include <setjmp.h>         // longjmp(), setjmp()
#endif  /* USE_OBFUSCATED_FLOW */
#include <stdio.h>          // printf()
#include <stdlib.h>         // exit(), EXIT_FAILURE, EXIT_SUCCESS


#ifdef USE_OBFUSCATED_FLOW
#define TASK_COUNT 3
jmp_buf envs[TASK_COUNT];
enum task
{
    TASK_A,
    TASK_B,
    TASK_C
};
enum task current_task;
#endif  /* USE_OBFUSCATED_FLOW */


void task_a(void);
void task_b(void);
void task_c(void);


int main(void)
{
    // LOCAL VARIABLES
    int results = EXIT_SUCCESS;  // Results of execution

    // EXECUTE TASKS
#ifdef USE_OBFUSCATED_FLOW
    if (0 == setjmp(envs[TASK_A]))
    {
        // Initial entry point
        current_task = TASK_A;
        longjmp(envs[TASK_A], 1);
    }
    else if (0 == setjmp(envs[TASK_B]))
    {
        results = EXIT_SUCCESS;
    }
    else if (0 == setjmp(envs[TASK_C]))
    {
        results = EXIT_SUCCESS;
    }

    if (EXIT_SUCCESS == results)
    {
        switch (current_task)
        {
            case TASK_A:
                task_a();
                break;
            case TASK_B:
                task_b();
                break;
            case TASK_C:
                task_c();
                break;
        }
    }
#else
    // Normal execution
    task_a();
    task_b();
    task_c();
#endif  /* USE_OBFUSCATED_FLOW */

    // DONE
    exit(results);
}


void task_a(void)
{
    printf("Task A executed.\n");
#ifdef USE_OBFUSCATED_FLOW
    current_task = TASK_B;
    longjmp(envs[TASK_B], EXIT_FAILURE);
#endif  /* USE_OBFUSCATED_FLOW */
}


void task_b(void)
{
    printf("Task B executed.\n");
#ifdef USE_OBFUSCATED_FLOW
    current_task = TASK_C;
    longjmp(envs[TASK_C], EXIT_FAILURE);
#endif  /* USE_OBFUSCATED_FLOW */
}


void task_c(void)
{
    printf("Task C executed.\n");
    // End of obfuscated flow.
}
