#ifndef __SKID_ASSEMBLY__
#define __SKID_ASSEMBLY__

#include <stdint.h>             // uint64_t

/*
 *  Description:
 *      Read the processor's timestamp counter, or equivalent, based on architecture.  Utilizes
 *      inline assembly.  Unsupported architectures will result in a pre-processor phase error.
 *      Supported architectures:
 *          - Intel x86
 *          - Intel x86-64
 *
 *  Returns:
 *      Current processor timestamp on success.
 */
uint64_t read_cpu_tsc();

#endif  /* __SKID_ASSEMBLY__ */
