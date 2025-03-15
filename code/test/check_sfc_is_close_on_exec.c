/*
 *  Check unit test suit for skid_file_control.h's is_close_on_exec() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfc_is_close_on_exec.bin && \
code/dist/check_sfc_is_close_on_exec.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfc_is_close_on_exec.bin

 *
 *  The test cases have been split up by normal, error, boundary, and special (NEBS).
 *  Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sfc_is_close_on_exec.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sfc_is_close_on_exec.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sfc_is_close_on_exec.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sfc_is_close_on_exec.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>                      // START_TEST(), END_TEST
#include <errno.h>                      // EINVAL
#include <fcntl.h>                      // O_CLOEXEC
#include <stdlib.h>                     // EXIT_FAILURE, EXIT_SUCCESS
// Local includes
#include "devops_code.h"                // resolve_to_repo(), SKID_REPO_NAME
#ifndef SKID_DEBUG
#define SKID_DEBUG
#endif  /* SKID_DEBUG */
// #include "skid_debug.h"                 // FPRINTF_ERR()
#include "skid_file_control.h"          // is_close_on_exec()
#include "skid_file_descriptors.h"      // close_fd(), open_fd()
#include "skid_validation.h"            // validate_skid_fd()
#include "unit_test_code.h"             // globals, setup(), teardown()

// Use this with printf("%s", BOOL_STR_LIT(bool)); to print human readable results
#define BOOL_STR_LIT(boolean) (boolean ? "true" : "false")
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value


/**************************************************************************************************/
/************************************ HELPER CODE DECLARATION *************************************/
/**************************************************************************************************/


/*
 *  Create the Check test suite.
 */
Suite *create_test_suite(void);

/*
 *  Call open_test_fd() and verify success.
 */
int get_test_fd(const char *filename, bool cloexec);

/*
 *  Open filename using open_fd().  If cloexec is true, the O_CLOEXEC flag will be used.
 */
int open_test_fd(const char *filename, bool cloexec, int *errnum);

/*
 *  Make the function call, check the expected return value, and validate the results.
 */
void run_test_case_fd(int input_fd, bool exp_return, int exp_errnum);

/*
 *  Open filename using open_fd(), make the function call, check the expected return value,
 *  validate the results, and close the open file descriptor.
 */
void run_test_case_fn(const char *filename, bool cloexec, bool exp_return, int exp_errnum);


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_file_not_cloexec)
{
    // LOCAL VARIABLES
    bool exp_return = false;   // Expected return value for this test case
    int exp_errnum = ENOERR;   // Expected errnum value for this test case
    bool use_cloexec = false;  // O_CLOEXEC usage for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_file_path;

    // RUN TEST
    run_test_case_fn(input_abs_path, use_cloexec, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_n02_file_cloexec)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    bool use_cloexec = true;  // O_CLOEXEC usage for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_file_path;

    // RUN TEST
    run_test_case_fn(input_abs_path, use_cloexec, exp_return, exp_errnum);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_bad_fd_negative)
{
    // LOCAL VARIABLES
    bool exp_return = false;  // Expected return value for this test case
    int exp_errnum = EBADF;   // Expected errnum value for this test case
    int input_fd = -90;       // Test input file descriptor

    // RUN TEST
    run_test_case_fd(input_fd, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_e02_bad_fd_large)
{
    // LOCAL VARIABLES
    bool exp_return = false;  // Expected return value for this test case
    int exp_errnum = EBADF;   // Expected errnum value for this test case
    int input_fd = 90;        // Test input file descriptor

    // RUN TEST
    run_test_case_fd(input_fd, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_e03_closed_fd)
{
    // LOCAL VARIABLES
    bool exp_return = false;     // Expected return value for this test case
    int exp_errnum = EBADF;      // Expected errnum value for this test case
    int input_fd = SKID_BAD_FD;  // Test input file descriptor
    bool use_cloexec = true;     // O_CLOEXEC usage for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_file_path;

    // RUN TEST
    // Open the file
    input_fd = get_test_fd(input_abs_path, use_cloexec);
    // Close the file descriptor
    ck_assert_msg(0 == close(input_fd), "The call to close(%d) failed\n", input_fd);
    // Run it
    run_test_case_fd(input_fd, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_e04_null_errnum)
{
    // LOCAL VARIABLES
    bool exp_return = false;      // Expected return value for this test case
    bool actual_ret = true;       // Actual return value from the function call
    int input_fd = SKID_BAD_FD;   // Test input file descriptor
    bool use_cloexec = true;      // O_CLOEXEC usage for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_file_path;

    // RUN TEST
    // Open the file
    input_fd = get_test_fd(input_abs_path, use_cloexec);
    // Call the function
    actual_ret = is_close_on_exec(input_fd, NULL);
    ck_assert_msg(exp_return == actual_ret, "is_close_on_exec(%d, NULL) returned '%s' "
                  "instead of '%s'\n", input_fd, BOOL_STR_LIT(actual_ret),
                  BOOL_STR_LIT(exp_return));
    // Cleanup
    close_fd(&input_fd, true);  // Best effort to close the file descriptor
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_s01_dir)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    bool use_cloexec = true;  // O_CLOEXEC usage for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_dir_path;

    // RUN TEST
    run_test_case_fn(input_abs_path, use_cloexec, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_s02_symlink)
{
    // LOCAL VARIABLES
    bool exp_return = true;   // Expected return value for this test case
    int exp_errnum = ENOERR;  // Expected errnum value for this test case
    bool use_cloexec = true;  // O_CLOEXEC usage for this test case
    // Absolute path for test input as resolved against the repo name
    char *input_abs_path = test_sym_link;

    // RUN TEST
    run_test_case_fn(input_abs_path, use_cloexec, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_s03_stdin)
{
    // LOCAL VARIABLES
    bool exp_return = false;       // Expected return value for this test case
    int exp_errnum = ENOERR;       // Expected errnum value for this test case
    int input_fd = SKID_STDIN_FD;  // Test input file descriptor

    // RUN TEST
    run_test_case_fd(input_fd, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_s04_stdout)
{
    // LOCAL VARIABLES
    bool exp_return = false;        // Expected return value for this test case
    int exp_errnum = ENOERR;        // Expected errnum value for this test case
    int input_fd = SKID_STDOUT_FD;  // Test input file descriptor

    // RUN TEST
    run_test_case_fd(input_fd, exp_return, exp_errnum);
}
END_TEST


START_TEST(test_s05_stderr)
{
    // LOCAL VARIABLES
    bool exp_return = false;        // Expected return value for this test case
    int exp_errnum = ENOERR;        // Expected errnum value for this test case
    int input_fd = SKID_STDERR_FD;  // Test input file descriptor

    // RUN TEST
    run_test_case_fd(input_fd, exp_return, exp_errnum);
}
END_TEST


/**************************************************************************************************/
/************************************* HELPER CODE DEFINITION *************************************/
/**************************************************************************************************/


Suite *create_test_suite(void)
{
    // LOCAL VARIABLES
    Suite *suite = suite_create("SFC_Is_Close_On_Exec");  // Test suite
    TCase *tc_normal = tcase_create("Normal");            // Normal test cases
    TCase *tc_error = tcase_create("Error");              // Error test cases
    TCase *tc_special = tcase_create("Special");          // Special test cases

    // SETUP TEST CASES
    tcase_add_checked_fixture(tc_normal, setup, teardown);
    tcase_add_checked_fixture(tc_error, setup, teardown);
    tcase_add_checked_fixture(tc_special, setup, teardown);
    tcase_add_test(tc_normal, test_n01_file_not_cloexec);
    tcase_add_test(tc_normal, test_n02_file_cloexec);
    tcase_add_test(tc_error, test_e01_bad_fd_negative);
    tcase_add_test(tc_error, test_e02_bad_fd_large);
    tcase_add_test(tc_error, test_e03_closed_fd);
    tcase_add_test(tc_error, test_e04_null_errnum);
    tcase_add_test(tc_special, test_s01_dir);
    tcase_add_test(tc_special, test_s02_symlink);
    tcase_add_test(tc_special, test_s03_stdin);
    tcase_add_test(tc_special, test_s04_stdout);
    tcase_add_test(tc_special, test_s05_stderr);
    suite_add_tcase(suite, tc_normal);
    suite_add_tcase(suite, tc_error);
    suite_add_tcase(suite, tc_special);

    // DONE
    return suite;
}


int get_test_fd(const char *filename, bool cloexec)
{
    // LOCAL VARIABLES
    int input_fd = SKID_BAD_FD;  // File descriptor for filename
    int result = CANARY_INT;     // Results of the call to open_test_fd()

    // OPEN IT
    input_fd = open_test_fd(filename, cloexec, &result);
    ck_assert_msg(ENOERR == result, "open_test_fd(%s, %s, %p) failed with errno "
                  "[%d] '%s'\n", filename, BOOL_STR_LIT(cloexec), &result, result,
                  strerror(result));
    ck_assert_msg(ENOERR == validate_skid_fd(input_fd), "open_test_fd(%s, %s, %p) returned "
                  "an invalid file descriptor of %d\n", filename, BOOL_STR_LIT(cloexec), &result,
                  input_fd);

    // DONE
    return input_fd;
}


int open_test_fd(const char *filename, bool cloexec, int *errnum)
{
    // LOCAL VARIABLES
    int fd = SKID_BAD_FD;  // Opened file descriptor
    int result = ENOERR;   // Results of the call to open_fd()
    int flags = 0;         // See open_fd()
    mode_t mode = 0;       // See open_fd()

    // OPEN IT
    if (true == cloexec)
    {
        flags = flags | O_CLOEXEC;
    }
    fd = open_fd(filename, flags, mode, &result);

    // DONE
    if (NULL != errnum)
    {
        *errnum = result;
    }
    return fd;
}


void run_test_case_fd(int input_fd, bool exp_return, int exp_errnum)
{
    // LOCAL VARIABLES
    bool actual_ret = 0;          // Return value of the tested function
    int actual_err = CANARY_INT;  // The actual errnum value from the function call

    // RUN IT
    // Call the function
    actual_ret = is_close_on_exec(input_fd, &actual_err);
    // Compare actual return value to expected return value
    ck_assert_msg(exp_return == actual_ret, "is_close_on_exec(%d, %p) returned '%s' "
                  "instead of '%s'\n", input_fd, &actual_err, BOOL_STR_LIT(actual_ret),
                  BOOL_STR_LIT(exp_return));
    // Comprare the errnum value to the expected errnum value
    // Verify the canary value was overwritten
    ck_assert_msg(CANARY_INT != actual_err, "is_close_on_exec(%d, %p) failed to overwrite"
                  " the errnum canary value\n", input_fd, &actual_err);
    ck_assert_msg(exp_errnum == actual_err, "is_close_on_exec(%d, %p) provided errno [%d] '%s' "
                  "instead of [%d] '%s'\n", input_fd, &actual_err, actual_err,
                  strerror(actual_err), exp_errnum, strerror(exp_errnum));

    // DONE
    return;
}


void run_test_case_fn(const char *filename, bool cloexec, bool exp_return, int exp_errnum)
{
    // LOCAL VARIABLES
    int input_fd = SKID_BAD_FD;  // File descriptor for filename

    // OPEN IT
    input_fd = get_test_fd(filename, cloexec);

    // RUN IT
    run_test_case_fd(input_fd, exp_return, exp_errnum);

    // DONE
    close_fd(&input_fd, true);  // Quietly close the file descriptor
}


int main(void)
{
    // LOCAL VARIABLES
    int errnum = 0;  // Errno from the function call
    // Relative path for this test case's input
    char log_rel_path[] = { "./code/test/test_output/check_sfc_is_close_on_exec.log" };
    // Absolute path for log_rel_path as resolved against the repo name
    char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
    int number_failed = 0;
    Suite *suite = NULL;
    SRunner *suite_runner = NULL;

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
