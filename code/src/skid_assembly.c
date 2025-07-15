/*
 *  TESTING NOTES:
 *      - To verify the conditional compiliation against the current architecture:
 *      Copy/paste the following...

gcc -E -I code/include/ -o skid_assembly.i code/src/skid_assembly.c  # less skid_assembly.i

 *
 *      - Preprocess the file for a specific architecture:
 *          1. undef the supported macros
 *          2. define the architecture macro to test for
 *          3. preprocess the source.
 *      Copy/paste the following...

gcc -E -U __x86_64__ -U __i386__ -U __aarch64__ -U __riscv -U __powerpc__ -U __powerpc64__ \
-D __i386__ -I code/include/ -o skid_assembly.i code/src/skid_assembly.c  # less skid_assembly.i

 *
 */

#define SKID_DEBUG                          // Turn on DEBUGGING

#include <errno.h>                          // EBADFD, EINVAL
#include "skid_assembly.h"                  // uint64_t
#include "skid_debug.h"                     // DEBUG logging
#include "skid_macros.h"                    // SKID_BAD_FD

MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/

/*
 *  Invokes the x86_64 write system call and returns its return value.
 */
static inline ssize_t call_write_x86_64(int fildes, const void *buf, size_t nbyte)
{
    // LOCAL VARIABLES
    ssize_t num_bytes = -1;  // Number of bytes written by the call to write

#if defined(__x86_64__) // Intel x86 Family
    // WRITE IT
    __asm__ __volatile__ ("movq $1, %%rax\nsyscall"
                          : "=a" (num_bytes)
                          : "D" (fildes), "S" (buf), "d" (nbyte)
                          : "rcx", "r11", "memory");
#endif  /* __x86_64__ */

    // DONE
    return num_bytes;
}


/*
 *  Reads and returns the PowerPC entire time base register on success, 0 on error.
 */
static inline uint64_t read_cpu_tsc_powerpc_fam(void)
{
    // LOCAL VARIABLES
    uint64_t tsc_val = 0;  // CPU's timestamp counter value
#if defined(__powerpc__) || defined(__powerpc64__)  // PowerPC Family
    uint32_t tbl = 0;      // Time Base Register (lower value)
    uint32_t tbu0 = 0;     // Time Base Register (upper value) temp variable
    uint32_t tbu1 = 0;     // Time Base Register (upper value)

    // READ IT
    do
    {
        __asm__ __volatile__("mftbu %0" : "=r"(tbu0));
        __asm__ __volatile__("mftb  %0" : "=r"(tbl));
        __asm__ __volatile__("mftbu %0" : "=r"(tbu1));
    } while (tbu0 != tbu1);
    tsc_val = ((uint64_t)tbu1 << 32) | tbl;  // Smush the bits together
#endif  /* __powerpc__ || __powerpc64__ */

    // DONE
    return tsc_val;
}


/*
 *  Reads and returns the Intel x86-family timestamp counter on success, 0 on error.
 *
 *  rdtsc - Reads the current value of the processor’s time-stamp counter (a 64-bit MSR) into the
 *      EDX:EAX registers. The EDX register is loaded with the high-order 32 bits of the MSR and
 *      the EAX register is loaded with the low-order 32 bits. (On processors that support the
 *      Intel 64 architecture, the high-order 32 bits of each of RAX and RDX are cleared.)
 */
static inline uint64_t read_cpu_tsc_x86_fam(void)
{
    // LOCAL VARIABLES
    uint64_t tsc_val = 0;        // CPU's timestamp counter value
#if defined(__i386__) || defined(__x86_64__) // Intel x86 Family
    uint32_t lo_order_bits = 0;  // Low-order 32 bits of the MSR from the EAX register
    uint32_t hi_order_bits = 0;  // High-order 32 bits of the MSR from the EDX register

    // READ IT
    /*
     *  Why call lfence first?
     *  The Pentium Pro and Pentium II processors support out-of-order execution instructions may
     *  be executed in another order as you programmed them. This can be a source of errors if
     *  not taken care of.  To prevent this the programmer must serialize the the instruction
     *  queue. This can be done by inserting a serializing instruction like CPUID instruction
     *  before the RDTSC instruction.
     */
    __asm__ __volatile__ ("lfence\n\trdtsc\n\t" : "=a"(lo_order_bits), "=d"(hi_order_bits));
    tsc_val = ((uint64_t)hi_order_bits << 32) | lo_order_bits;  // Smush the bits together
#endif  /* __i386__ || __x86_64__ */

    // DONE
    return tsc_val;
}


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/

/*
    INTERNAL NOTES:
    6.11.2 Extended Asm - Assembler Instructions with C Expression Operands
        asm asm-qualifiers ( AssemblerTemplate
                              : OutputOperands
                              : InputOperands
                              : Clobbers
                              : GotoLabels)

    Qualifiers:
        volatile - The typical use of extended asm statements is to manipulate input values to
            produce output values. However, your asm statements may also produce side effects.
            If so, you may need to use the volatile qualifier to disable certain optimizations.


    The asm keyword is a GNU extension. When writing code that can be compiled with -ansi and the
        various -std options, use __asm__ instead of asm (see Alternate Keywords).

    6.11.2.3 Output Operands
    [ [asmSymbolicName] ] constraint (cvariablename)

    6.11.3.3 Constraint Modifier Characters
    ‘=’ Means that this operand is written to by this instruction: the previous value is discarded
        and replaced by new data.
    For x86 family Machine Constraints (config/i386/constraints.md)
        `a` The a register.
        `d` The d register.
    For x86_64
        `D` RDI
        `S` RSI
        `d` RDX
 */

ssize_t call_write(int fildes, const void *buf, size_t nbyte)
{
    // LOCAL VARIABLES
    ssize_t num_bytes = -1;  // Return value from the write syscall

    // INPUT VALIDATION
    if (SKID_BAD_FD == fildes || fildes < 0)
    {
        PRINT_ERROR(Received a known-bad file descriptor);
        PRINT_ERRNO(EBADFD);
    }
    else if (NULL == buf)
    {
        PRINT_ERROR(Received a NULL pointer);
        PRINT_ERRNO(EINVAL);
    }
    else if (0 >= nbyte)
    {
        PRINT_ERROR(Received a NULL pointer);
        PRINT_ERRNO(EINVAL);
    }
    else
    {
    // SYSTEM VALIDATION
#ifdef __GNUC__
#if defined(__x86_64__)  // Intel x86-64
        FPRINTF_ERR("%s %s supports Intel x86-64\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
        num_bytes = call_write_x86_64(fildes, buf, nbyte);
#else
#error "This function does not support the current architecture."
#endif  /* Built-in Architecture Macros */
#else
#error "This function does not support non-GNU compatible compilers."
#endif  /* __GNUC__ */
    }

    // DONE
    return num_bytes;
}


uint64_t read_cpu_tsc()
{
    // LOCAL VARIABLES
    uint64_t tsc_val = 0;        // CPU's timestamp counter value

    // SYSTEM VALIDATION
#ifdef __GNUC__
#if defined(__x86_64__)  // Intel x86-64
    FPRINTF_ERR("%s %s supports Intel x86-64\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
    tsc_val = read_cpu_tsc_x86_fam();
#elif defined(__i386__)  // Intel x86
    FPRINTF_ERR("%s %s supports Intel x86\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
    tsc_val = read_cpu_tsc_x86_fam();
#elif defined(__aarch64__)  // AArch64 (ARM64)
    FPRINTF_ERR("%s %s supports AArch64\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(tsc_val));
#elif defined(__riscv)  // RISC-V
    FPRINTF_ERR("%s %s supports RISC-V\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
     __asm__ __volatile__("rdcycle %0" : "=r"(tsc_val));
#elif defined(__powerpc__)  // PowerPC
    FPRINTF_ERR("%s %s supports PowerPC\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
    tsc_val = read_cpu_tsc_powerpc_fam();
#elif defined(__powerpc64__)  // PowerPC64
    FPRINTF_ERR("%s %s supports PowerPC64\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
    tsc_val = read_cpu_tsc_powerpc_fam();
#else
#error "This function does not support the current architecture."
#endif  /* Built-in Architecture Macros */
#else
#error "This function does not support non-GNU compatible compilers."
#endif  /* __GNUC__ */

    // DONE
    return tsc_val;
}
