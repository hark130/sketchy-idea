/*
 *  Check unit test suit for skid_file_metadata_read.h's get_access_time() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmr_get_access_time.bin
code/dist/check_sfmr_get_access_time.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmr_get_access_time.bin

 *
 */

#include <check.h>                    // START_TEST(), END_TEST
#include <limits.h>                   // PATH_MAX
#include <stdio.h>                    // printf()
#include <stdlib.h>
#include <unistd.h>                   // get_current_dir_name()
// Local includes
#include "devops_code.h"              // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"  // get_access_time()
#include "unit_test_code.h"           // globals, setup(), teardown()


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
 *	Get the expected return value for pathname.
 */
time_t get_expected_return(const char *pathname);

/*
 *  Call the function and verify the return value and errnum value.
 */
void run_test_case(const char *pathname, bool follow_sym, time_t exp_return, int exp_errnum);


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_block_device)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                     // Expected results
	int exp_errnum = 0;                        // Expected errnum
	char input_abs_path[] = { "/dev/loop0" };  // Test case input
	bool follow_sym = true;                    // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_n02_character_device)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                    // Expected results
	int exp_errnum = 0;                       // Expected errnum
	char input_abs_path[] = { "/dev/null" };  // Test case input
	bool follow_sym = true;                   // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_n03_directory)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                 // Expected results
	int exp_errnum = 0;                    // Expected errnum
	char *input_abs_path = test_dir_path;  // Test case input
	bool follow_sym = true;                // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                  // Expected results
	int exp_errnum = 0;                     // Expected errnum
	char *input_abs_path = test_pipe_path;  // Test case input
	bool follow_sym = true;                 // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_n05_regular_file)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                  // Expected results
	int exp_errnum = 0;                     // Expected errnum
	char *input_abs_path = test_file_path;  // Test case input
	bool follow_sym = true;                 // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_n06_socket)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                    // Expected results
	int exp_errnum = 0;                       // Expected errnum
	char *input_abs_path = test_socket_path;  // Test case input
	bool follow_sym = true;                   // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
	// LOCAL VARIABLES
    ino_t exp_result = 0;                     // Expected return value
    int exp_errnum = 0;                       // Expected errnum value
    char *input_abs_path = test_sym_link;     // Absolute filename of the symlink
    char *actual_abs_path = test_file_path;   // Actual path the symlink points to
	bool follow_sym = true;                   // Test case input

	// SETUP
	exp_result = get_expected_return(actual_abs_path);

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_filename)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;        // Expected results
	int exp_errnum = EINVAL;      // Expected errnum
	char *input_abs_path = NULL;  // Test case input
	bool follow_sym = true;       // Test case input

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_e02_empty_filename)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                  // Expected results
	int exp_errnum = EINVAL;                // Expected errnum
	char *input_abs_path = "\0 NOT HERE!";  // Test case input
	bool follow_sym = true;                 // Test case input

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


START_TEST(test_e03_null_errnum)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                // Expected results
	time_t act_result = 0;                // Actual result
	char *input_abs_path = "/dev/loop0";  // Test case input
	bool follow_sym = true;               // Test case input

	// TEST START
	act_result = get_access_time(input_abs_path, NULL, follow_sym);
	ck_assert_msg(act_result == exp_result, "get_access_time() returned %ld instead of %ld",
				  act_result, exp_result);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_s01_missing_filename)
{
	// LOCAL VARIABLES
	time_t exp_result = 0;                         // Expected results
	int exp_errnum = ENOENT;                       // Expected errnum
	char *input_abs_path = "/does/not/exist.txt";  // Test case input
	bool follow_sym = true;                        // Test case input

	// TEST START
	run_test_case(input_abs_path, follow_sym, exp_result, exp_errnum);
}
END_TEST


/**************************************************************************************************/
/************************************* HELPER CODE DEFINITION *************************************/
/**************************************************************************************************/

 
Suite *create_test_suite(void)
{
    // LOCAL VARIABLES
	Suite *suite = suite_create("SFMR_Get_Access_Time");  // Test suite
    TCase *tc_normal = tcase_create("Normal");            // Normal test cases
    TCase *tc_error = tcase_create("Error");              // Error test cases
    // TCase *tc_boundary = tcase_create("Boundary");        // Boundary test cases
    TCase *tc_special = tcase_create("Special");          // Special test cases

	/* Core test case */
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


time_t get_expected_return(const char *pathname)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;                                 // Errno from the function call
	time_t exp_result = get_shell_atime(pathname, &errnum);  // Expected result

	// VALIDATION
	ck_assert_msg(0 == errnum, "get_shell_atime(%s) failed with [%d] %s",
				  pathname, errnum, strerror(errnum));

	// DONE
	return exp_result;
}


void run_test_case(const char *pathname, bool follow_sym, time_t exp_return, int exp_errnum)
{
    // LOCAL VARIABLES
    int act_errnum = CANARY_INT;  // Actual errno value from the call
    char *boolean = "FALSE";      // Indicates follow_sym
    // Actual return value from the call
    time_t act_return = get_access_time(pathname, &act_errnum, follow_sym);

    // SETUP
    if (true == follow_sym)
    {
		boolean = "TRUE";
    }

    // CHECK IT
	ck_assert_msg(act_return == exp_return, "get_access_time(%s, %s) returned %ld instead of %ld",
				  pathname, boolean, act_return, exp_return);
    ck_assert_msg(act_errnum == exp_errnum, "get_access_time(%s, %s) resulted in errnum [%d] '%s' "
                  "instead of [%d] '%s", pathname, boolean, act_errnum, strerror(act_errnum),
                  exp_errnum, strerror(exp_errnum));
}


int main(void)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno from the function call
	// Relative path for this test case's input
	char log_rel_path[] = { "./code/test/test_output/check_sfmr_get_access_time.log" };
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
