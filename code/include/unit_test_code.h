/*
 *	This library contains non-releasable, common-use Check unit test code.
 */

#ifndef __SKID_CHECK_CODE__
#define __SKID_CHECK_CODE__

// Local includes
#include "devops_code.h"                // SKID_REPO_NAME

#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value

/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

char *test_dir_path;  // Heap array containing the absolute directory name resolved to the repo
char *test_file_path;  // Heap array containing the absolute filename resolved to the repo
char *test_pipe_path;  // Heap array containing the absolute pipe filename resolved to the repo
char *test_socket_path;  // Heap array containing the absolute socket filename resolved to the repo
char *test_sym_link;  // Heap array with the absolute symbolic link filename resolved to the repo
char *test_dst_link;  // Heap array with the test cases's default destination symbolic link

/*
 *  Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *  the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *  Resolve the named pipe and raw socket default filenames to the repo and store the heap
 *  memory pointer in the globals.
 */
void setup(void);

/*
 *  Delete the named pipe and raw socket files.  Then, free the heap memory arrays.
 */
void teardown(void);

#endif  /* __SKID_CHECK_CODE__ */
