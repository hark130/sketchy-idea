/*
 *  This source file was created for the express purpose of showcasing gcc options.
 *  It explicitly defines its own custom entry point.  It also uses inline assembly to
 *  make certain system calls since it's not being linked against a library.
 *
 *  Copy/paste the following...

gcc -nostdlib -o ./code/dist/showcase_gcc_nostdlib.bin ./code/test/showcase_gcc_nostdlib.c -static && \
./code/dist/showcase_gcc_nostdlib.bin

 *
 */

#include <stddef.h>                     // size_t
#include <unistd.h>                     // ssize_t

#define MESSAGE "Hello from showcase_gcc_nostdlib.bin\n"


ssize_t call_write(int fildes, const void *buf, size_t nbyte)
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


void _start()
{
    char buf[] = { MESSAGE };  // String to write
    call_write(1, buf, sizeof(buf) - 1);

#if defined(__x86_64__)  // Intel x86-64
    __asm__ volatile ("mov $60, %%rax\n"
                      "xor %%rdi, %%rdi\n"
                      "syscall\n"
                      ::: "%rax", "%rdi");
#else
#error "This function does not support the current architecture."
#endif  /* Built-in Architecture Macros */
}
