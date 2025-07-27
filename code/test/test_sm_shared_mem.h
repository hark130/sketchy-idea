/*
 *	Defines a SPOT for common variables between the shared memory server and client.
 *	Intended for use by:
 *		- test_sm_shared_mem_server.c
 *		- test_sm_shared_mem_client.c
 */

#ifndef __TEST_SKID_SHARED_MEM__
#define __TEST_SKID_SHARED_MEM__

/* SHARED MEMORY MACROS */
// The mode to use for the shared memory object
#define SHM_MODE SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | \
                 SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | \
                 SKID_MODE_OTHER_R | SKID_MODE_OTHER_W  // rw-rw-rw-
#define SHM_NAME "/t_sm_s_m_shm"                   // The shared memory object's name
#define BUF_LEN 1024                               // Length of the mapped memory buffer
#define BUF_SIZE (size_t)(BUF_LEN * sizeof(char))  // Size, in bytes, of the mapped memory buffer

/* NAMED SEMAPHORE MACROS */
#define SEM_NAME "/t_sm_s_m_sem"                   // The named semaphore

#endif  /* __TEST_SKID_SHARED_MEM__ */
