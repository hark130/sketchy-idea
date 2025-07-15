#ifndef __SKID_ASSEMBLY__
#define __SKID_ASSEMBLY__

#include <stdint.h>             // uint64_t
#include <unistd.h>             // ssize_t

/*
 *  Description:
 *      Call the write syscall using inline assembly.
 *      Unsupported architectures will result in an error during the pre-processor phase.
 *
 *      | ------------ | --------- | ------ |
 *      | Architecture | Supported | Tested |
 *      | ------------ | --------- | ------ |
 *      | AArch64      |           |        |
 *      | Intel x86    |           |        |
 *      | Intel x86-64 | ✓         | ✓      |
 *      | PowerPC      |           |        |
 *      | PowerPC64    |           |        |
 *      | RISC-V       |           |        |
 *
 *  Returns:
 *      Returns the number of bytes written, which may be less than nbyte, on success.
 *      Returns -1 on failure.
 */
ssize_t call_write(int fildes, const void *buf, size_t nbyte);

/*
 *  Description:
 *      Read the processor's timestamp counter, or equivalent, based on architecture.  Utilizes
 *      inline assembly.  Unsupported architectures will result in a pre-processor phase error.
 *
 *      | ------------ | --------- | ------ |
 *      | Architecture | Supported | Tested |
 *      | ------------ | --------- | ------ |
 *      | AArch64      | ✓         |        |
 *      | Intel x86    | ✓         |        |
 *      | Intel x86-64 | ✓         | ✓      |
 *      | PowerPC      | ✓         |        |
 *      | PowerPC64    | ✓         |        |
 *      | RISC-V       | ✓         |        |
 *
 *  Returns:
 *      Current processor timestamp on success.
 */
uint64_t read_cpu_tsc();

#endif  /* __SKID_ASSEMBLY__ */
