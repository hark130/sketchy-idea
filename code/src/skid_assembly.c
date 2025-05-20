
#define SKID_DEBUG  // Turn on DEBUGGING

#include "skid_assembly.h"  // uint64_t
#include "skid_debug.h"     // DEBUG logging


uint64_t read_cpu_tsc()
{
    // LOCAL VARIABLES
    uint64_t timestamp = 0;  // CPU's timestamp counter value

    // SYSTEM VALIDATION
#if defined(__x86_64__)  // Intel x86-64
    FPRINTF_ERR("%s %s supports Intel x86-64\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
#elif define(__i386__)  // Intel x86
    FPRINTF_ERR("%s %s supports Intel x86\n", DEBUG_INFO_STR, __FUNCTION_NAME__);
#else
#error "This function does not support the current architecture."
#endif  /* Built-in Architecture Macros */

    // DONE
    return timestamp;
}

