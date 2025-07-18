/*
 *  Check unit test suit for skid_file_metadata_read.h's is_path() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmr_is_path.bin && \
code/dist/check_sfmr_is_path.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmr_is_path.bin

 *
 *  The test cases have been split up by normal, error, boundary, and special (NEBS).
 *  Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sfmr_is_path.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sfmr_is_path.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sfmr_is_path.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sfmr_is_path.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#ifndef SKID_DEBUG
#define SKID_DEBUG
#endif  /* SKID_DEBUG */

#include <check.h>                      // START_TEST(), END_TEST, Suite
#include <errno.h>                      // EINVAL, ENOENT
#include <stdlib.h>                     // EXIT_FAILURE, EXIT_SUCCESS
// Local includes
#include "devops_code.h"                // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"    // is_path()
#include "skid_macros.h"                // ENOERR
#include "unit_test_code.h"             // BOOL_STR_LIT(), CANARY_INT, globals, setup(), teardown()


/**************************************************************************************************/
/************************************ HELPER CODE DECLARATION *************************************/
/**************************************************************************************************/

/*
 *  Create the Check test suite.
 */
Suite *create_test_suite(void);

/*
 *  Make the function call, check the expected return value, check the expected errnum value,
 *  and validate the results.
 */
void run_test_case(const char *pathname, bool exp_return, int exp_errnum);


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_block_device)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/dev/loop0";

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n02_character_device)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/dev/null";

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n03_directory)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_dir_path;

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_pipe_path;

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n05_regular_file)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_file_path;

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n06_socket)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_socket_path;

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_sym_link;

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n08_missing_dir)
{
    // LOCAL VARIABLES
    bool exp_return = false;  // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/does/not/exist/";

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n09_missing_file)
{
    // LOCAL VARIABLES
    bool exp_return = false;  // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/tmp/missing_file.bak";

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_pathname)
{
    // LOCAL VARIABLES
    bool exp_return = false;  // Expected return value for this test case
    int exp_errnum = EINVAL;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = NULL;

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_e02_empty_pathname)
{
    // LOCAL VARIABLES
    bool exp_return = false;  // Expected return value for this test case
    int exp_errnum = EINVAL;  // Expected errnum value for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "\0 NOT HERE!";

    // RUN TEST
    run_test_case(input_abs_path, exp_return, exp_errnum);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


// START_TEST(test_s01_deadend_sym_link)
// {
//     // LOCAL VARIABLES
//     bool exp_return = true;   // Expected return value for this test case
//     int exp_errnum = EINVAL;  // Expected errnum value for this test case
//     // Absolute path for test input as resolved against the repo name
//     char *input_abs_path = "TO DO: DON'T DO NOW";

//     // RUN TEST
//     run_test_case(input_abs_path, exp_return, exp_errnum);
// }
// END_TEST


/**************************************************************************************************/
/************************************* HELPER CODE DEFINITION *************************************/
/**************************************************************************************************/


Suite *create_test_suite(void)
{
    // LOCAL VARIABLES
    Suite *suite = suite_create("SFMR_Is_Path");  // Test suite
    TCase *tc_normal = tcase_create("Normal");    // Normal test cases
    TCase *tc_error = tcase_create("Error");      // Error test cases
    // TCase *tc_special = tcase_create("Special");  // Special test cases

    // SETUP TEST CASES
    tcase_add_checked_fixture(tc_normal, setup, teardown);
    tcase_add_checked_fixture(tc_error, setup, teardown);
    // tcase_add_checked_fixture(tc_special, setup, teardown);
    tcase_add_test(tc_normal, test_n01_block_device);
    tcase_add_test(tc_normal, test_n02_character_device);
    tcase_add_test(tc_normal, test_n03_directory);
    tcase_add_test(tc_normal, test_n04_named_pipe);
    tcase_add_test(tc_normal, test_n05_regular_file);
    tcase_add_test(tc_normal, test_n06_socket);
    tcase_add_test(tc_normal, test_n07_symbolic_link);
    tcase_add_test(tc_normal, test_n08_missing_dir);
    tcase_add_test(tc_normal, test_n09_missing_file);
    tcase_add_test(tc_error, test_e01_null_pathname);
    tcase_add_test(tc_error, test_e02_empty_pathname);
    // tcase_add_test(tc_special, test_s01_deadend_sym_link);
    suite_add_tcase(suite, tc_normal);
    suite_add_tcase(suite, tc_error);
    // suite_add_tcase(suite, tc_special);

    // DONE
    return suite;
}


void run_test_case(const char *pathname, bool exp_return, int exp_errnum)
{
    // LOCAL VARIABLES
    bool actual_ret = false;         // Return value of the tested function
    int actual_errnum = CANARY_INT;  // Actual errnum value of the tested function

    // RUN IT
    // Call the function
    actual_ret = is_path(pathname, &actual_errnum);
    // Compare actual return value to expected return value
    ck_assert_msg(exp_return == actual_ret, "is_path(%s) returned '%s' instead of '%s'\n",
                  pathname, BOOL_STR_LIT(actual_ret), BOOL_STR_LIT(exp_return));
    // Verify actual errnum value was changed
    ck_assert_msg(CANARY_INT != actual_errnum, "is_path(%s) did not change the errnum value\n",
                  pathname);
    // Compare actual errnum value to expected errnum value
    ck_assert_msg(exp_errnum == actual_errnum, "is_path(%s) reported errnum as [%d] '%s' "
                  "instead of [%d] '%s'\n", pathname,
                  actual_errnum, strerror(actual_errnum), exp_errnum, strerror(exp_errnum));

    // DONE
    return;
}


int main(void)
{
    // LOCAL VARIABLES
    int errnum = 0;                // Errno from the function call
    int number_failed = 0;         // Number of test cases that failed
    Suite *suite = NULL;           // Test suite
    SRunner *suite_runner = NULL;  // Test suite runner
    // Relative path for this test case's input
    char log_rel_path[] = { "./code/test/test_output/check_sfc_is_close_on_exec.log" };
    // Absolute path for log_rel_path as resolved against the repo name
    char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);

    // SETUP
    suite = create_test_suite();
    suite_runner = srunner_create(suite);
    srunner_set_log(suite_runner, log_abs_path);

    // RUN IT
    srunner_run_all(suite_runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(suite_runner);

    // CLEANUP
    srunner_free(suite_runner);
    free_devops_mem((void **)&log_abs_path);

    // DONE
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
