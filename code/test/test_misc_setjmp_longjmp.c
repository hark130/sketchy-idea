/*
 *  Source file to demonstrate setjmp()/longjmp() usage.
 *
 *  Copy/paste these compiler commands to build a clean and obfuscated version of this code:
 *

gcc -I ./code/include/ -o ./code/dist/test_misc_setjmp_longjmp_clean.bin ./code/test/test_misc_setjmp_longjmp.c && ./code/dist/test_misc_setjmp_longjmp_clean.bin; echo $?
gcc -I ./code/include/ -o ./code/dist/test_misc_setjmp_longjmp_obfus.bin ./code/test/test_misc_setjmp_longjmp.c -DUSE_OBFUSCATED_FLOW && ./code/dist/test_misc_setjmp_longjmp_obfus.bin; echo $?

 *
 *  OBFUSCAATED CONTROL FLOW (OCF)
 *  The "clean" control flow couldn't be simpler.
 *  I'll describe the "obfuscated" flow here.  Everything else should be self-evident.
 *  01. `0 == setjmp(envs[TASK_A])` resolves to true
 *  02. `longjmp(envs[TASK_A], EXIT_FAILURE)` jumps back to "setjmp(envs[TASK_A])"
 *  03. `0 == setjmp(envs[TASK_A])` resolves to false (because it returns EXIT_FAILURE instead)
 *  04. `0 == setjmp(envs[TASK_B])` resolves to true
 *  05. "task_a()" executes and jumps back to "setjmp(envs[TASK_B])"
 *  06. `0 == setjmp(envs[TASK_B])` resolves to false (because it returns EXIT_FAILURE instead)
 *  07. `0 == setjmp(envs[TASK_C])` resolves to true
 *  08. "task_b()" executes and jumps back to "setjmp(envs[TASK_C])"
 *  09. `0 == setjmp(envs[TASK_C])` resolves to false (because it returns EXIT_FAILURE instead)
 *     and falls through the if/else-if.
 *  10. "task_c()" executes and returns to "main()"
 *  11. "main()" `exit(results)`s
 *
 *  Copy/paste these commands to compare static analysis between the two binaries:
 *

objdump -d ./code/dist/test_misc_setjmp_longjmp_clean.bin
objdump -d ./code/dist/test_misc_setjmp_longjmp_obfus.bin

 *
 */


#include <setjmp.h>         // longjmp(), setjmp()
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


// Malware Analogy: Staged payloads in a trapdoor delivery system.
void task_a(void);  // Initial Access Stage
void task_b(void);  // Decryption or Unpacking Stage
void task_c(void);  // Execution Stage / Final Payload


int main(void)
{
    // LOCAL VARIABLES
    int results = EXIT_SUCCESS;  // Results of execution

    // EXECUTE TASKS
#ifdef USE_OBFUSCATED_FLOW
    if (0 == setjmp(envs[TASK_A]))  // OCF-01, OCF-03
    {
        // Initial entry point
        current_task = TASK_A;
        longjmp(envs[TASK_A], EXIT_FAILURE);  // OCF-02
    }
    else if (0 == setjmp(envs[TASK_B]))  // OCF-04, OCF-06
    {
        results = EXIT_SUCCESS;
    }
    else if (0 == setjmp(envs[TASK_C]))  // OCF-07, OCF-09
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
    exit(results);  // OCF-11
}


void task_a(void)
{
    printf("Task A executed.\n");
#ifdef USE_OBFUSCATED_FLOW
    current_task = TASK_B;
    longjmp(envs[TASK_B], EXIT_FAILURE);  // OCF-05
#endif  /* USE_OBFUSCATED_FLOW */
}


void task_b(void)
{
    printf("Task B executed.\n");
#ifdef USE_OBFUSCATED_FLOW
    current_task = TASK_C;
    longjmp(envs[TASK_C], EXIT_FAILURE);  // OCF-08
#endif  /* USE_OBFUSCATED_FLOW */
}


void task_c(void)
{
    printf("Task C executed.\n");  // OCF-10
    // End of obfuscated flow.
}
