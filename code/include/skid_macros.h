/*
 *    Defines common use macros for use with SKETCHY IDEA (SKID).
 */

#ifndef __SKID_MACROS__
#define __SKID_MACROS__

#include <sys/types.h>                      // pid_t

/* GENERAL MACROS */
// ENOERR
#ifndef ENOERR
    #define ENOERR ((int)0)  // Success value for errno
#endif  /* ENOERR */
// NULL
#ifndef NULL
    #define NULL ((void *)0)  // Just in case it's not already defined
#endif  /* NULL */
// SKID_MAX_SZ
#define SKID_MAX_SZ (~(size_t)0)    // Library's value for the maximum size_t value
// ENV*BIT
#ifdef __GNUC__
    #if defined(__x86_64__) || defined(__ppc64__)
        #define ENV64BIT
    #else
        #define ENV32BIT
    #endif
#endif  /* __GNUC__ */
// SKID_INTERNAL
#if (defined(__GNUC__) || defined(__clang__)) && defined(SKID_RELEASE)
#define SKID_INTERNAL __attribute__((visibility("internal")))
#else
#define SKID_INTERNAL
#endif  /* SKID_INTERNAL */

/* CLONE MACROS */
#define SKID_STACK_SIZE (8 * 1024)  // Default stack size for a cloned child

/* FILE MACROS */
#if defined(PATH_MAX)
    #define SKID_PATH_MAX PATH_MAX
#elif defined(FILENAME_MAX)
    #if defined(ENV64BIT)
        #if FILENAME_MAX <= 4096
            #define SKID_PATH_MAX FILENAME_MAX
        #else
            #define SKID_PATH_MAX 4096
        #endif
    #else
        #if FILENAME_MAX <= 1024
            #define SKID_PATH_MAX FILENAME_MAX
        #else
            #define SKID_PATH_MAX 1024
        #endif
    #endif  /* FILENAME_MAX */
#else
    #if defined(ENV64BIT)
        #define SKID_PATH_MAX 4096
    #else
        #define SKID_PATH_MAX 1024
    #endif
#endif

/* FILE DESCRIPTOR MACROS */
#define SKID_BAD_FD (signed int)-1  // Use this to standardize "invalid" file descriptors
// SKID_STDIN_FD - File number of stdin.
#ifdef STDIN_FILENO
#define SKID_STDIN_FD STDIN_FILENO
#else
#define SKID_STDIN_FD 0
#endif  /* SKID_STDIN_FD */
// SKID_STDOUT_FD - File number of stdout.
#ifdef STDOUT_FILENO
#define SKID_STDOUT_FD STDOUT_FILENO
#else
#define SKID_STDOUT_FD 1
#endif  /* SKID_STDOUT_FD */
// SKID_STDERR_FD - File number of stderr.
#ifdef STDERR_FILENO
#define SKID_STDERR_FD STDERR_FILENO
#else
#define SKID_STDERR_FD 2
#endif  /* SKID_STDERR_FD */

/* FILE METADATA MACROS */
// You may use these macros with SKID mode_t arguments.
/* We translated the chown(2) macros so you don't have to! */
// Owner Permissions
#define SKID_MODE_OWNER_R S_IRUSR  // Read by owner
#define SKID_MODE_OWNER_W S_IWUSR  // Write by owner
#define SKID_MODE_OWNER_X S_IXUSR  // Execute/search by owner
// Group Permissions
#define SKID_MODE_GROUP_R S_IRGRP  // Read by group
#define SKID_MODE_GROUP_W S_IWGRP  // Write by group
#define SKID_MODE_GROUP_X S_IXGRP  // Execute/search by group
// Other Permissions
#define SKID_MODE_OTHER_R S_IROTH  // Read by others
#define SKID_MODE_OTHER_W S_IWOTH  // Write by others
#define SKID_MODE_OTHER_X S_IXOTH  // Execute/search by others
// Special Permissions
#define SKID_MODE_SET_UID S_ISUID  // set-user-ID
#define SKID_MODE_SET_GID S_ISGID  // set-user-ID
#define SKID_MODE_STICKYB S_ISVTX  // sticky bit (see: unlink(2), restricted deletion flag)

/* NETWORK MACROS */
// This value is calculated from packet sizes but is not guaranteed to be successful
#define SKID_MAX_DGRAM_DATA_IPV4 65507  // Maximum UDP payload size, in bytes, over IPv4
// This value was based on the "maximum safe datagram payload size"
#define SKID_CHUNK_SIZE 508  // The default chunk size for any network send func w/ chunking
// This literal is used to translate the IPPROTO_RAW protocol number into an alias
#define SKID_RAW_SOCK_ALIAS "RAW"  // Use this alias to check for a raw socket protocol

/* PID MACROS */
#define SKID_BAD_PID (pid_t)-1  // Use this to standardize "invalid" PIDs

#endif  /* __SKID_MACROS__ */
