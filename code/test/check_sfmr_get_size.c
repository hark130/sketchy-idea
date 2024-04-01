/*
 *  Check unit test suit for skid_file_metadata_read.h's get_size() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmr_get_size.bin
code/dist/check_sfmr_get_size.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmr_get_size.bin

 *
 */

#include <check.h>                    // START_TEST(), END_TEST
#include <limits.h>                   // PATH_MAX
#include <stdio.h>                    // printf()
#include <stdlib.h>
#include <unistd.h>                   // get_current_dir_name()
// Local includes
#include "devops_code.h"              // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"  // get_size()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/
START_TEST(test_n01_block_device)
{
    // LOCAL VARIABLES
    off_t result = 0;                      // Return value from function call
    int errnum = CANARY_INT;               // Errno from the function call
    off_t exp_result = 0;                  // Expected results
    char input_path[] = { "/dev/loop0" };  // Test case input

    // SETUP
    exp_result = get_shell_size(input_path, &errnum);

    // VALIDATION
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n02_character_device)
{
    // LOCAL VARIABLES
    off_t result = 0;                     // Return value from function call
    int errnum = CANARY_INT;              // Errno from the function call
    off_t exp_result = 0;                 // Expected results
    char input_path[] = { "/dev/null" };  // Test case input

    // SETUP
    exp_result = get_shell_size(input_path, &errnum);

    // VALIDATION
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n03_directory)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    off_t result = 0;                        // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    off_t exp_result = 0;                    // Expected results
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    exp_result = get_shell_size(input_abs_path, &errnum);
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_abs_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    off_t result = 0;                        // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    off_t exp_result = 0;                    // Expected results
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/named_pipe" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, false, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    remove_a_file(input_abs_path, true);  // Remove leftovers and ignore errors
    // It is important make_a_pipe() succeeds
    errnum = make_a_pipe(input_abs_path);
    ck_assert_msg(0 == errnum, "make_a_pipe(%s) failed with [%d] %s", input_abs_path,
                  errnum, strerror(errnum));
    exp_result = get_shell_size(input_abs_path, &errnum);
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_abs_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    remove_a_file(input_abs_path, true);
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n05_regular_file)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    off_t result = 0;                        // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    off_t exp_result = 0;                    // Expected results
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/regular_file.txt" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    exp_result = get_shell_size(input_abs_path, &errnum);
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_abs_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n06_socket)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;                     // Repo name
    off_t result = 0;                                           // Return value from function call
    int errnum = CANARY_INT;                                    // Errno from the function call
    off_t exp_result = 0;                                       // Expected results
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/raw_socket" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, false, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    remove_a_file(input_abs_path, true);  // Remove leftovers and ignore errors
    // It is important make_a_socket() succeeds
    errnum = make_a_socket(input_abs_path);
    ck_assert_msg(0 == errnum, "make_a_socket(%s) failed with [%d] %s", input_abs_path,
                  errnum, strerror(errnum));
    exp_result = get_shell_size(input_abs_path, &errnum);
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_abs_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    remove_a_file(input_abs_path, true);
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    off_t result = 0;                        // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    off_t exp_result = 0;                    // Expected results
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/sym_link.txt" };
    // Relative path for this test case's input
    char actual_rel_path[] = { "./code/test/test_input/regular_file.txt" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);
    // Absolute path for actual_rel_path as resolved against the repo name
    char *actual_abs_path = resolve_to_repo(repo_name, actual_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    exp_result = get_shell_size(actual_abs_path, &errnum);  // Special because of sym link
    // It is important get_shell_size() succeeds
    ck_assert_msg(0 == errnum, "get_shell_size() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_size() provided an invalid value of %ld",
                  exp_result);
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = get_size(input_abs_path, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
    free_devops_mem(&actual_abs_path);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/
START_TEST(test_e01_null_filename)
{
    off_t result = 0;         // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    off_t exp_result = 0;     // Expected results
    result = get_size(NULL, &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e02_empty_filename)
{
    off_t result = 0;         // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    off_t exp_result = 0;     // Expected results
    result = get_size("\0 NOT HERE!", &errnum);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e03_null_errnum)
{
    off_t result = 0;         // Return value from function call
    off_t exp_result = 0;     // Expected results
    result = get_size("/dev/loop0", NULL);
    ck_assert_msg(exp_result == result, "get_size() returned %ld instead of %ld",
                  result, exp_result);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/
START_TEST(test_s01_missing_filename)
{
    off_t result = 0;         // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = get_size("/does/not/exist.txt", &errnum);
    ck_assert(0 == result);
    ck_assert_int_eq(ENOENT, errnum);
}
END_TEST

 
Suite *get_size_suite(void)
{
    Suite *suite = NULL;
    TCase *tc_core = NULL;

    suite = suite_create("SFMR_Get_Size");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_n01_block_device);
    tcase_add_test(tc_core, test_n02_character_device);
    tcase_add_test(tc_core, test_n03_directory);
    tcase_add_test(tc_core, test_n04_named_pipe);
    tcase_add_test(tc_core, test_n05_regular_file);
    tcase_add_test(tc_core, test_n06_socket);
    tcase_add_test(tc_core, test_n07_symbolic_link);
    tcase_add_test(tc_core, test_e01_null_filename);
    tcase_add_test(tc_core, test_e02_empty_filename);
    tcase_add_test(tc_core, test_e03_null_errnum);
    tcase_add_test(tc_core, test_s01_missing_filename);
    suite_add_tcase(suite, tc_core);

    return suite;
}


int main(void)
{
    // LOCAL VARIABLES
    int errnum = 0;  // Errno from the function call
    // Relative path for this test case's input
    char log_rel_path[] = { "./code/test/test_output/check_sfmr_get_size.log" };
    // Absolute path for log_rel_path as resolved against the repo name
    char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
    int number_failed = 0;
    Suite *suite = NULL;
    SRunner *suite_runner = NULL;

    // SETUP
    suite = get_size_suite();
    suite_runner = srunner_create(suite);
    srunner_set_log(suite_runner, log_abs_path);

    // RUN IT
    srunner_run_all(suite_runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(suite_runner);

    // CLEANUP
    srunner_free(suite_runner);
    free_devops_mem(&log_abs_path);

    // DONE
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
