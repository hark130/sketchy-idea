/*
 *    This library contains non-releasable, common-use Check unit test code.
 */

#ifndef __SKID_UNIT_TEST_CODE__
#define __SKID_UNIT_TEST_CODE__

// Local includes
#include "devops_code.h"                // SKID_REPO_NAME

// Use this with printf("%s", BOOL_STR_LIT(bool)); to print human readable results
#define BOOL_STR_LIT(boolean) (boolean ? "true" : "false")
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value

/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

extern char *test_dir_path;     // Heap array with the absolute directory name resolved to the repo
extern char *test_file_path;    // Heap array with the absolute filename resolved to the repo
extern char *test_pipe_path;    // Heap array with the absolute pipe filename resolved to the repo
extern char *test_socket_path;  // Heap array with the absolute socket filename resolved to the repo
extern char *test_sym_link;     // Heap array with the repo's absolute symbolic link file
extern char *test_dst_link;     // Heap array with the default absolute destination symbolic link

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

#endif  /* __SKID_UNIT_TEST_CODE__ */
