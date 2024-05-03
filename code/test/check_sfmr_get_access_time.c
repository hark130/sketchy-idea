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


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value


/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/


char *test_pipe_path;  // Heap array containing the absolute pipe filename resolved to the repo
char *test_socket_path;  // Heap array containing the absolute socket filename resolved to the repo

/*
 *	Get the expected return value for pathname.
 */
time_t get_expected_return(const char *pathname);

/*
 *	Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *	the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *	Resolve the named pipe and raw socket default filenames to the repo and store the heap
 *	memory pointer in the globals.
 */
void setup(void);

/*
 *	Delete the named pipe and raw socket files.  Then, free the heap memory arrays.
 */
void teardown(void);


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


char *resolve_test_input(const char *pathname)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;                 // Errno values
	const char *repo_name = SKID_REPO_NAME;  // Name of the repo
	char *resolved_name = NULL;              // pathname resolved to repo_name

	// RESOLVE IT
	resolved_name = resolve_to_repo(repo_name, pathname, false, &errnum);
	ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
				  pathname, errnum, strerror(errnum));
	ck_assert_msg(NULL != resolved_name, "resolve_to_repo(%s, %s) failed to resolve the path",
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
	int errnum = CANARY_INT;                                      // Errno values
	char named_pipe[] = { "./code/test/test_input/named_pipe" };  // Default test input: pipe
	char raw_socket[] = { "./code/test/test_input/raw_socket" };  // Default test input: socket

	// SET IT UP
	// Named Pipe
	test_pipe_path = resolve_test_input(named_pipe);
	if (test_pipe_path)
	{
		remove_a_file(test_pipe_path, true);  // Remove leftovers and ignore errors
		errnum = make_a_pipe(test_pipe_path);
		ck_assert_msg(0 == errnum, "make_a_pipe(%s) failed with [%d] %s", test_pipe_path,
					  errnum, strerror(errnum));
		errnum = CANARY_INT;  // Reset temp variable
	}
	errnum = CANARY_INT;  // Reset temp variable
	// Raw Socket
	test_socket_path = resolve_test_input(raw_socket);
	if (test_socket_path)
	{
		remove_a_file(test_socket_path, true);  // Remove leftovers and ignore errors
		errnum = make_a_socket(test_socket_path);
		ck_assert_msg(0 == errnum, "make_a_socket(%s) failed with [%d] %s", test_socket_path,
					  errnum, strerror(errnum));
		errnum = CANARY_INT;  // Reset temp variable
	}

	// DONE
	return;
}


void teardown(void)
{
	// Pipe
	remove_a_file(test_pipe_path, true);  // Best effort
	free_devops_mem((void **)&test_pipe_path);  // Ignore any errors
	// Socket
	remove_a_file(test_socket_path, true);  // Best effort
	free_devops_mem((void **)&test_socket_path);  // Ignore any errors
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/
START_TEST(test_n01_block_device)
{
	// LOCAL VARIABLES
	time_t result = 0;                         // Return value from function call
	int errnum = CANARY_INT;                   // Errno from the function call
	time_t exp_result = 0;                     // Expected results
	char input_abs_path[] = { "/dev/loop0" };  // Test case input
	bool follow_sym = true;                    // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	result = get_access_time(input_abs_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n02_character_device)
{
	// LOCAL VARIABLES
	time_t result = 0;                        // Return value from function call
	int errnum = CANARY_INT;                  // Errno from the function call
	time_t exp_result = 0;                    // Expected results
	char input_abs_path[] = { "/dev/null" };  // Test case input
	bool follow_sym = true;                    // Test case input

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	result = get_access_time(input_abs_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n03_directory)
{
	// LOCAL VARIABLES
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function calls
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input
	// Absolute path for input_rel_path as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/");

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	result = get_access_time(input_abs_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
	// LOCAL VARIABLES
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function calls
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input

	// SETUP
	exp_result = get_expected_return(test_pipe_path);

	// TEST START
	result = get_access_time(test_pipe_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n05_regular_file)
{
	// LOCAL VARIABLES
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function calls
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input
	// Absolute path for input_rel_path as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/regular_file.txt");

	// SETUP
	exp_result = get_expected_return(input_abs_path);

	// TEST START
	result = get_access_time(input_abs_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_n06_socket)
{
	// LOCAL VARIABLES
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function calls
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input

	// SETUP
	exp_result = get_expected_return(test_socket_path);

	// TEST START
	result = get_access_time(test_socket_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
	// LOCAL VARIABLES
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function calls
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input
	// Absolute path for input_rel_path as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/sym_link.txt");
	// Absolute path for actual_rel_path as resolved against the repo name
	char *actual_abs_path = resolve_test_input("./code/test/test_input/regular_file.txt");

	// SETUP
	exp_result = get_expected_return(actual_abs_path);

	// TEST START
	result = get_access_time(input_abs_path, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
	free_devops_mem((void **)&actual_abs_path);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/
START_TEST(test_e01_null_filename)
{
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function call
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input
	result = get_access_time(NULL, &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e02_empty_filename)
{
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function call
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input
	result = get_access_time("\0 NOT HERE!", &errnum, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
	ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e03_null_errnum)
{
	time_t result = 0;        // Return value from function call
	time_t exp_result = 0;    // Expected results
	bool follow_sym = true;   // Test case input
	result = get_access_time("/dev/loop0", NULL, follow_sym);
	ck_assert_msg(exp_result == result, "get_access_time() returned %ld instead of %ld",
				  result, exp_result);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/
START_TEST(test_s01_missing_filename)
{
	time_t result = 0;        // Return value from function call
	int errnum = CANARY_INT;  // Errno from the function call
	bool follow_sym = true;   // Test case input
	result = get_access_time("/does/not/exist.txt", &errnum, follow_sym);
	ck_assert(0 == result);
	ck_assert_int_eq(ENOENT, errnum);
}
END_TEST

 
Suite *get_access_time_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_core = NULL;

	suite = suite_create("SFMR_Get_Access_Time");

	/* Core test case */
	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
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
	char log_rel_path[] = { "./code/test/test_output/check_sfmr_get_access_time.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = get_access_time_suite();
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
