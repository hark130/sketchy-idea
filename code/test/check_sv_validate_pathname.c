/*
 *  Check unit test suit for skid_validation.h's validate_pathname() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sv_validate_pathname.bin
code/dist/check_sv_validate_pathname.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sv_validate_pathname.bin

 *
 *  The test cases have been split up by normal, error, boundary, and special (NEBS).
 *  Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sv_validate_pathname.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sv_validate_pathname.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sv_validate_pathname.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sv_validate_pathname.bin; unset CK_RUN_CASE  # Just run the Special test cases

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
#include "skid_macros.h"                // ENOERR
#include "skid_validation.h"            // validate_pathname()
#include "unit_test_code.h"             // BOOL_STR_LIT(), CANARY_INT, globals, setup(), teardown()


/**************************************************************************************************/
/************************************ HELPER CODE DECLARATION *************************************/
/**************************************************************************************************/

/*
 *  Create the Check test suite.
 */
Suite *create_test_suite(void);

/*
 *  Make the function call, check the expected return value, and validate the results.
 */
void run_test_case(const char *pathname, bool must_exist, int exp_return);


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_block_device)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/dev/loop0";

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_n02_character_device)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/dev/null";

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_n03_directory)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_dir_path;

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_pipe_path;

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_n05_regular_file)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_file_path;

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_n06_socket)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_socket_path;

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_sym_link;

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_filename)
{
    // LOCAL VARIABLES
    int exp_return = EINVAL;  // Expected return value for this test case
    bool must_exist = false;  // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = NULL;

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_e02_empty_filename)
{
    // LOCAL VARIABLES
    int exp_return = EINVAL;  // Expected return value for this test case
    bool must_exist = false;  // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "\0 NOT HERE!";

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_s01_missing_filename_must_exist)
{
    // LOCAL VARIABLES
    int exp_return = ENOENT;  // Expected return value for this test case
    bool must_exist = true;   // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/does/not/exist.txt";

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


START_TEST(test_s02_missing_filename_optional)
{
    // LOCAL VARIABLES
    int exp_return = ENOERR;  // Expected return value for this test case
    bool must_exist = false;  // Controls lstat() usage
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = "/does/not/exist.txt";

    // RUN TEST
    run_test_case(input_abs_path, must_exist, exp_return);
}
END_TEST


/**************************************************************************************************/
/************************************* HELPER CODE DEFINITION *************************************/
/**************************************************************************************************/


Suite *create_test_suite(void)
{
    // LOCAL VARIABLES
    Suite *suite = suite_create("SV_Validate_Pathname");  // Test suite
    TCase *tc_normal = tcase_create("Normal");            // Normal test cases
    TCase *tc_error = tcase_create("Error");              // Error test cases
    TCase *tc_special = tcase_create("Special");          // Special test cases

    // SETUP TEST CASES
    tcase_add_checked_fixture(tc_normal, setup, teardown);
    tcase_add_checked_fixture(tc_error, setup, teardown);
    tcase_add_checked_fixture(tc_special, setup, teardown);
    tcase_add_test(tc_normal, test_n01_block_device);
    tcase_add_test(tc_normal, test_n02_character_device);
    tcase_add_test(tc_normal, test_n03_directory);
    tcase_add_test(tc_normal, test_n04_named_pipe);
    tcase_add_test(tc_normal, test_n05_regular_file);
    tcase_add_test(tc_normal, test_n06_socket);
    tcase_add_test(tc_normal, test_n07_symbolic_link);
    tcase_add_test(tc_error, test_e01_null_filename);
    tcase_add_test(tc_error, test_e02_empty_filename);
    tcase_add_test(tc_special, test_s01_missing_filename_must_exist);
    tcase_add_test(tc_special, test_s02_missing_filename_optional);
    suite_add_tcase(suite, tc_normal);
    suite_add_tcase(suite, tc_error);
    suite_add_tcase(suite, tc_special);

    // DONE
    return suite;
}


void run_test_case(const char *pathname, bool must_exist, int exp_return)
{
    // LOCAL VARIABLES
    int actual_ret = CANARY_INT;  // Return value of the tested function

    // RUN IT
    // Call the function
    actual_ret = validate_skid_pathname(pathname, must_exist);
    // Compare actual return value to expected return value
    ck_assert_msg(exp_return == actual_ret, "validate_pathname(%s, %s) returned [%d] '%s' "
                  "instead of [%d] '%s'\n", pathname, BOOL_STR_LIT(must_exist),
                  actual_ret, strerror(actual_ret), exp_return, strerror(exp_return));

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
