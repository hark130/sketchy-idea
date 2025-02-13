/*
 *  Check unit test suit for skid_file_metadata_read.h's get_serial_num() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmr_get_serial_num.bin
code/dist/check_sfmr_get_serial_num.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmr_get_serial_num.bin

 *
 */

#include <check.h>                    // START_TEST(), END_TEST
#include <limits.h>                   // PATH_MAX
#include <stdio.h>                    // printf()
#include <stdlib.h>
#include <unistd.h>                   // get_current_dir_name()
// Local includes
#include "devops_code.h"              // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"  // get_serial_num()
#include "unit_test_code.h"           // globals, resolve_test_input(), setup(), teardown()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value


/**************************************************************************************************/
/************************************ HELPER CODE DECLARATION *************************************/
/**************************************************************************************************/

/*
 *  Create the Check test suite.
 */
Suite *create_test_suite(void);

/*
 *  Description:
 *      Uses get_shell_inode() to determine the expected result.
 *
 *  Args:
 *      pathname: The pathname, relative or absolute, to fetch the inode number for.
 *
 *  Returns:
 *      Inode number on success.  If there's an error, errnum will be set with the actual errno
 *      value.  On success, errnum will be zeroized.
 */
ino_t get_expected_result(const char *pathname);

/*
 *  Call the function and verify the return value and errnum value.
 */
void run_test_case(const char *pathname, int exp_return, int exp_errno);


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_block_device)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                  // Expected return value
    int exp_errno = 0;                     // Expected errnum value
    char input_path[] = { "/dev/loop0" };  // Test case input

    // SETUP
    exp_result = get_expected_result(input_path);

    // TEST START
    run_test_case(input_path, exp_result, exp_errno);
}
END_TEST


START_TEST(test_n02_character_device)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                 // Expected return value
    int exp_errno = 0;                    // Expected errnum value
    char input_path[] = { "/dev/null" };  // Test case input

    // SETUP
    exp_result = get_expected_result(input_path);

    // TEST START
    run_test_case(input_path, exp_result, exp_errno);
}
END_TEST


START_TEST(test_n03_directory)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                  // Expected return value
    int exp_errno = 0;                     // Expected errnum value
    char *input_abs_path = test_dir_path;  // Test case input

    // SETUP
    exp_result = get_expected_result(input_abs_path);

    // TEST START
    run_test_case(input_abs_path, exp_result, exp_errno);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                   // Expected return value
    int exp_errno = 0;                      // Expected errnum value
    char *input_abs_path = test_pipe_path;  // Abs path to the named pipe

    // SETUP
    exp_result = get_expected_result(input_abs_path);

    // TEST START
    run_test_case(input_abs_path, exp_result, exp_errno);
}
END_TEST


START_TEST(test_n05_regular_file)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                   // Expected return value
    int exp_errno = 0;                      // Expected errnum value
    char *input_abs_path = test_file_path;  // Abs path to the regular file

    // SETUP
    exp_result = get_expected_result(input_abs_path);

    // TEST START
    run_test_case(input_abs_path, exp_result, exp_errno);
}
END_TEST


START_TEST(test_n06_socket)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                     // Expected return value
    int exp_errno = 0;                        // Expected errnum value
    char *input_abs_path = test_socket_path;  // Abs path to the socket

    // SETUP
    exp_result = get_expected_result(input_abs_path);

    // TEST START
    run_test_case(input_abs_path, exp_result, exp_errno);
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
    // LOCAL VARIABLES
    ino_t exp_result = 0;                     // Expected return value
    int exp_errno = 0;                        // Expected errnum value
    char *input_abs_path = test_sym_link;     // Absolute filename of the symlink
    char *actual_abs_path = test_file_path;   // Actual path the symlink points to

    // SETUP
    exp_result = get_expected_result(actual_abs_path);

    // TEST START
    run_test_case(input_abs_path, exp_result, exp_errno);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_filename)
{
    ino_t result = 0;         // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    ino_t exp_result = 0;     // Expected results
    result = get_serial_num(NULL, &errnum);
    ck_assert_msg(exp_result == result, "get_serial_num() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e02_empty_filename)
{
    ino_t result = 0;         // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    ino_t exp_result = 0;     // Expected results
    result = get_serial_num("\0 NOT HERE!", &errnum);
    ck_assert_msg(exp_result == result, "get_serial_num() returned %ld instead of %ld",
                  result, exp_result);
    ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e03_null_errnum)
{
    ino_t result = 0;         // Return value from function call
    ino_t exp_result = 0;     // Expected results
    result = get_serial_num("/dev/loop0", NULL);
    ck_assert_msg(exp_result == result, "get_serial_num() returned %ld instead of %ld",
                  result, exp_result);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_s01_missing_filename)
{
    ino_t result = 0;         // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = get_serial_num("/does/not/exist.txt", &errnum);
    ck_assert(0 == result);
    ck_assert_int_eq(ENOENT, errnum);
}
END_TEST


/**************************************************************************************************/
/************************************* HELPER CODE DEFINITION *************************************/
/**************************************************************************************************/


Suite *create_test_suite(void)
{
    // LOCAL VARIABLES
    Suite *suite = suite_create("SFMR_Get_Serial_Num");  // Test suite
    TCase *tc_normal = tcase_create("Normal");           // Normal test cases
    TCase *tc_error = tcase_create("Error");             // Error test cases
    // TCase *tc_boundary = tcase_create("Boundary");       // Boundary test cases
    TCase *tc_special = tcase_create("Special");         // Special test cases

    // SETUP TEST CASES
    tcase_add_checked_fixture(tc_normal, setup, teardown);
    tcase_add_checked_fixture(tc_error, setup, teardown);
    // tcase_add_checked_fixture(tc_boundary, setup, teardown);
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
    tcase_add_test(tc_error, test_e03_null_errnum);
    tcase_add_test(tc_special, test_s01_missing_filename);
    suite_add_tcase(suite, tc_normal);
    suite_add_tcase(suite, tc_error);
    // suite_add_tcase(suite, tc_boundary);
    suite_add_tcase(suite, tc_special);

    return suite;
}


ino_t get_expected_result(const char *pathname)
{
    // LOCAL VARIABLES
    int errnum = 0;        // Errno from the function call
    ino_t exp_result = 0;  // Expected results

    // GET IT
    exp_result = get_shell_inode(pathname, &errnum);
    // It is important get_shell_inode() succeeds
    ck_assert_msg(0 == errnum, "get_shell_inode() failed with [%d] %s",
                  errnum, strerror(errnum));
    ck_assert_msg(exp_result >= 0, "get_shell_inode() provided an invalid value of %ld",
                  exp_result);

    // DONE
    return exp_result;
}


void run_test_case(const char *pathname, int exp_return, int exp_errnum)
{
    // LOCAL VARIABLES
    int act_errnum = CANARY_INT;                             // Actual errno value from the call
    int act_return = get_serial_num(pathname, &act_errnum);  // Actual return value from the call

    // CHECK IT
    ck_assert_msg(act_return == exp_return, "get_serial_num(%s) returned %ld instead of %ld",
                  pathname, act_return, exp_return);
    ck_assert_int_eq(exp_errnum, act_errnum);  // The out param should be zeroized on success
    ck_assert_msg(act_errnum == exp_errnum, "get_serial_num(%s) resulted in errnum [%d] '%s' "
                  "instead of [%d] '%s", pathname, act_errnum, strerror(act_errnum),
                  exp_errnum, strerror(exp_errnum));
}


int main(void)
{
    // LOCAL VARIABLES
    int errnum = 0;  // Errno from the function call
    // Relative path for this test case's input
    char log_rel_path[] = { "./code/test/test_output/check_sfmr_get_serial_num.log" };
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
