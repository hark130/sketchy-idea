/*
 *  Check unit test suit for skid_file_link.h's create_sym_link() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfl_create_sym_link.bin && \
code/dist/check_sfl_create_sym_link.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfl_create_sym_link.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sfl_create_sym_link.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sfl_create_sym_link.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sfl_create_sym_link.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sfl_create_sym_link.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>						// START_TEST(), END_TEST
#include <stdlib.h>
// Local includes
#include "devops_code.h"				// resolve_to_repo(), SKID_REPO_NAME
#ifndef SKID_DEBUG
#define SKID_DEBUG
#endif  /* SKID_DEBUG */
#include "skid_debug.h"    				// FPRINTF_ERR()
#include "skid_file_link.h"				// create_sym_link()
#include "skid_file_metadata_read.h"	// is_sym_link()

#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value

/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

char *test_dir_path;  // Heap array containing the absolute directory name resolved to the repo
char *test_file_path;  // Heap array containing the absolute filename resolved to the repo
char *test_pipe_path;  // Heap array containing the absolute pipe filename resolved to the repo
char *test_socket_path;  // Heap array containing the absolute socket filename resolved to the repo
char *test_sym_link;  // Heap array with the absolute symbolic link filename resolved to the repo
char *test_dst_link;  // Heap array with the test cases's default destination symbolic link

/*
 *	Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *	the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *	Make the function call, check the expected return value, and validate the results.
 */
void run_test_case(const char *src_input, const char *dst_input, int exp_return);

/*
 *	Resolve the named pipe and raw socket default filenames to the repo and store the heap
 *	memory pointer in the globals.
 */
void setup(void);

/*
 *	Delete the named pipe and raw socket files.  Then, free the heap memory arrays.
 */
void teardown(void);


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


void run_test_case(const char *src_input, const char *dst_input, int exp_return)
{
	// LOCAL VARIABLES
	int actual_ret = 0;  // Return value of the tested function
	int errnum = 0;      // Catch errno values here

	// RUN IT
	// Call the function
	actual_ret = create_sym_link(src_input, dst_input);
	// Compare actual results to expected results
	ck_assert_msg(exp_return == actual_ret, "create_sym_link(%s, %s) returned [%d] '%s' "
				  "instead of [%d] '%s'\n", src_input, dst_input, actual_ret,
				  strerror(actual_ret), exp_return, strerror(exp_return));
	// No need to check dst_input unless the test was expected to succeed
	if (0 == exp_return)
	{
		ck_assert_msg(true == is_sym_link(dst_input, &errnum),
			          "'%s' did *not* register as a symbolic link", dst_input);
		ck_assert_msg(0 == errnum, "is_sym_link(%s) failed with [%d] '%s'\n",
			          dst_input, errnum, strerror(errnum));
	}

	// DONE
	return;
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


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_directory)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_dir_path;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_n02_named_pipe)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_pipe_path;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_n03_regular_file)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_path;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_n04_socket)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_socket_path;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_n05_symbolic_link)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_sym_link;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_src_pathname)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = NULL;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_e02_empty_src_pathname)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char input_abs_path[] = { "\0 NOT HERE!" };
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_e03_null_link_pathname)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_path;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = NULL;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_e04_empty_link_pathname)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_path;
	// Absolute path for test output as resolved against the repo name
	char output_abs_path[] = { "\0 NOT HERE EITHER!" };

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_e05_both_null)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = NULL;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = NULL;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_e06_both_empty)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char input_abs_path[] = { "\0 NOT HERE!" };
	// Absolute path for test output as resolved against the repo name
	char output_abs_path[] = { "\0 NOT HERE EITHER!" };

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_s01_missing_filename)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char input_abs_path[] = { "/file/not/found" };
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


// Symbolic links can point to themselves but the OS will warn, "Too many levels of symbolic links"
START_TEST(test_s02_infinite_symbolic_link)
{
	// LOCAL VARIABLES
	int exp_return = 0;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_dst_link;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_dst_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


START_TEST(test_s03_destination_sym_link_exists)
{
	// LOCAL VARIABLES
	int exp_return = EEXIST;  // Expected return value for this test case
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_path;
	// Absolute path for test output as resolved against the repo name
	char *output_abs_path = test_sym_link;

	// RUN TEST
	run_test_case(input_abs_path, output_abs_path, exp_return);
}
END_TEST


Suite *create_sym_link_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SFL_Create_Sym_Link");  // Test suite
	TCase *tc_normal = tcase_create("Normal");           // Normal test cases
	TCase *tc_error = tcase_create("Error");             // Error test cases
	TCase *tc_special = tcase_create("Special");         // Special test cases

	// SETUP TEST CASES
	tcase_add_checked_fixture(tc_normal, setup, teardown);
	tcase_add_checked_fixture(tc_error, setup, teardown);
	tcase_add_checked_fixture(tc_special, setup, teardown);
	tcase_add_test(tc_normal, test_n01_directory);
	tcase_add_test(tc_normal, test_n02_named_pipe);
	tcase_add_test(tc_normal, test_n03_regular_file);
	tcase_add_test(tc_normal, test_n04_socket);
	tcase_add_test(tc_normal, test_n05_symbolic_link);
	tcase_add_test(tc_error, test_e01_null_src_pathname);
	tcase_add_test(tc_error, test_e02_empty_src_pathname);
	tcase_add_test(tc_error, test_e03_null_link_pathname);
	tcase_add_test(tc_error, test_e04_empty_link_pathname);
	tcase_add_test(tc_error, test_e05_both_null);
	tcase_add_test(tc_error, test_e06_both_empty);
	tcase_add_test(tc_special, test_s01_missing_filename);
	tcase_add_test(tc_special, test_s02_infinite_symbolic_link);
	tcase_add_test(tc_special, test_s03_destination_sym_link_exists);
	suite_add_tcase(suite, tc_normal);
	suite_add_tcase(suite, tc_error);
	suite_add_tcase(suite, tc_special);

	// DONE
	return suite;
}


int main(void)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno from the function call
	// Relative path for this test case's input
	char log_rel_path[] = { "./code/test/test_output/check_sfl_create_sym_link.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = create_sym_link_suite();
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
