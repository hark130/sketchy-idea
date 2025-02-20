/*
 *    This library contains non-releasable, common-use Check unit test code.
 */


#include <check.h>                        // Check assert messages
#include <string.h>                        // strerror()
// Local includes
#include "devops_code.h"                // free_devops_mem(), make_a_pipe(), make_a_socket(),
                                        // remove_a_file(), resolve_to_repo()
#include "unit_test_code.h"                // function declarations, globals


char *test_dir_path;     // Heap array with the absolute directory name resolved to the repo
char *test_file_path;    // Heap array with the absolute filename resolved to the repo
char *test_pipe_path;    // Heap array with the absolute pipe filename resolved to the repo
char *test_socket_path;  // Heap array with the absolute socket filename resolved to the repo
char *test_sym_link;     // Heap array with the repo's absolute symbolic link file
char *test_dst_link;     // Heap array with the default absolute destination symbolic link


char *resolve_test_input(const char *pathname)
{
    // LOCAL VARIABLES
    int errnum = CANARY_INT;                 // Errno values
    const char *repo_name = SKID_REPO_NAME;  // Name of the repo
    char *resolved_name = NULL;              // pathname resolved to repo_name

    // RESOLVE IT
    resolved_name = resolve_to_repo(repo_name, pathname, false, &errnum);
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s\n", repo_name,
                  pathname, errnum, strerror(errnum));
    ck_assert_msg(NULL != resolved_name, "resolve_to_repo(%s, %s) failed to resolve the path\n",
                  repo_name, pathname);

    // DONE
    if (0 != errnum && resolved_name)
    {
        free_devops_mem((void **)&resolved_name);  // Best effort
    }
    return resolved_name;
}


void setup(void)
{
    // LOCAL VARIABLES
    int errnum = CANARY_INT;                                          // Errno values
    char directory[] = { "./code/test/test_input/" };                 // Default test input: dir
    char named_pipe[] = { "./code/test/test_input/named_pipe" };      // Default test input: pipe
    char raw_socket[] = { "./code/test/test_input/raw_socket" };      // Default test input: socket
    char reg_file[] = { "./code/test/test_input/regular_file.txt" };  // Default test input: file
    char sym_link[] = { "./code/test/test_input/sym_link.txt" };      // Default test input: symlink
    char dst_link[] = { "./code/test/test_output/dst_link.txt" };     // Default test destination

    // SET IT UP
    // Directory
    test_dir_path = resolve_test_input(directory);
    // Named Pipe
    test_pipe_path = resolve_test_input(named_pipe);
    if (test_pipe_path)
    {
        remove_a_file(test_pipe_path, true);  // Remove leftovers and ignore errors
        errnum = make_a_pipe(test_pipe_path);
        ck_assert_msg(0 == errnum, "make_a_pipe(%s) failed with [%d] %s\n", test_pipe_path,
                      errnum, strerror(errnum));
        errnum = CANARY_INT;  // Reset temp variable
    }
    // Raw Socket
    test_socket_path = resolve_test_input(raw_socket);
    if (test_socket_path)
    {
        remove_a_file(test_socket_path, true);  // Remove leftovers and ignore errors
        errnum = make_a_socket(test_socket_path);
        ck_assert_msg(0 == errnum, "make_a_socket(%s) failed with [%d] %s\n", test_socket_path,
                      errnum, strerror(errnum));
        errnum = CANARY_INT;  // Reset temp variable
    }
    // Regular File
    test_file_path = resolve_test_input(reg_file);
    // Symbolic Link
    test_sym_link = resolve_test_input(sym_link);
    if (test_sym_link)
    {
        remove_a_file(test_sym_link, true);  // Remove leftovers and ignore errors
        errnum = make_a_symlink(test_file_path, test_sym_link);
        ck_assert_msg(0 == errnum, "make_a_symlink(%s, %s) failed with [%d] %s\n", test_file_path,
                      test_sym_link, errnum, strerror(errnum));
        errnum = CANARY_INT;  // Reset temp variable
    }
    // Destination Symbolic Link
    test_dst_link = resolve_test_input(dst_link);

    // DONE
    return;
}


void teardown(void)
{
    // Directory
    free_devops_mem((void **)&test_dir_path);  // Ignore any errors
    // File
    free_devops_mem((void **)&test_file_path);  // Ignore any errors
    // Pipe
    remove_a_file(test_pipe_path, true);  // Best effort
    free_devops_mem((void **)&test_pipe_path);  // Ignore any errors
    // Socket
    remove_a_file(test_socket_path, true);  // Best effort
    free_devops_mem((void **)&test_socket_path);  // Ignore any errors
    // Symbolic Link
    free_devops_mem((void **)&test_sym_link);  // Ignore any errors
    // Destination Symbolic Link
    remove_a_file(test_dst_link, true);  // Best effort
    free_devops_mem((void **)&test_dst_link);  // Ignore any errors
}
