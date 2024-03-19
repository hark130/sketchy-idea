/*
 *  Check unit test suit for skip_file_metadata_read.h's is_regular_file() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmr_is_regular_file.bin
code/dist/check_sfmr_is_regular_file.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmr_is_regular_file.bin

 *
 */

#include <check.h>                    // START_TEST(), END_TEST
#include <limits.h>                   // PATH_MAX
#include <stdio.h>                    // printf()
#include <stdlib.h>
#include <unistd.h>                   // get_current_dir_name()
// Local includes
#include "devops_code.h"              // resolve_to_repo(), SKIP_REPO_NAME
#include "skip_file_metadata_read.h"  // is_character_device()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value


START_TEST(test_n01_block_device)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_regular_file("/dev/loop0", &errnum);
    ck_assert(false == result);  // This input is *NOT* a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n02_character_device)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_regular_file("/dev/null", &errnum);
    ck_assert(false == result);  // This input is *NOT* a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n03_directory)
{
    // LOCAL VARIABLES
    const char *repo_name = SKIP_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_regular_file(input_abs_path, &errnum);
    ck_assert(false == result);  // This input is *NOT* a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n04_regular_file)
{
    // LOCAL VARIABLES
    const char *repo_name = SKIP_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/regular_file.txt" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_regular_file(input_abs_path, &errnum);
    ck_assert(true == result);  // This input is a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n05_symbolic_link)
{
    // LOCAL VARIABLES
    const char *repo_name = SKIP_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/sym_link.txt" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_regular_file(input_abs_path, &errnum);
    ck_assert(true == result);  // This may not be a regular file but stat() follows links
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST

 
Suite *is_regular_file_suite(void)
{
    Suite *suite = NULL;
    TCase *tc_core = NULL;

    suite = suite_create("Is_Regular_File");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_n01_block_device);
    tcase_add_test(tc_core, test_n02_character_device);
    tcase_add_test(tc_core, test_n03_directory);
    tcase_add_test(tc_core, test_n04_regular_file);
    tcase_add_test(tc_core, test_n05_symbolic_link);
    suite_add_tcase(suite, tc_core);

    return suite;
}


int main(void)
{
    int number_failed = 0;
    Suite *suite = NULL;
    SRunner *suite_runner = NULL;

    suite = is_regular_file_suite();
    suite_runner = srunner_create(suite);

    srunner_run_all(suite_runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
