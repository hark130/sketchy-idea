/*
 *  Check unit test suit for skid_dir_operations.h's create_dir() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sdo_create_dir.bin && \
code/dist/check_sdo_create_dir.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sdo_create_dir.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sdo_create_dir.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sdo_create_dir.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sdo_create_dir.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sdo_create_dir.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>						// START_TEST(), END_TEST
#include <stdlib.h>						// EXIT_FAILURE, EXIT_SUCCESS
#include <linux/limits.h>				// PATH_MAX
#include <stdio.h>						// sprintf()
// Local includes
#include "devops_code.h"				// resolve_to_repo(), SKID_REPO_NAME
#include "skid_dir_operations.h"		// create_dir()
#include "skid_file_metadata_read.h"	// is_directory()
#include "skid_macros.h"				// SKID_MODE_* macros

#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
#define WORKING_DIR "/tmp"  // The working directory for these test cases
#define BASE_DIR_NAME "create_dir"  // Incorporate this into the auto-generated directory names
// Try to replicate check/src/check_pack.c's get_max_msg_size() results
#ifndef DEFAULT_MAX_MSG_SIZE
#define DEFAULT_MAX_MSG_SIZE 4096  // Try to avoid check_pack.c "Message string too long" errors
#endif  /* DEFAULT_MAX_MSG_SIZE */
// Common Mode Macros
#define TEST_MODE_0111 (SKID_MODE_OWNER_X | SKID_MODE_GROUP_X | SKID_MODE_OTHER_X)
#define TEST_MODE_0220 (SKID_MODE_OWNER_W | SKID_MODE_GROUP_W )
#define TEST_MODE_0222 (TEST_MODE_0220 | SKID_MODE_OTHER_W)
#define TEST_MODE_0333 (TEST_MODE_0222 | TEST_MODE_0111)
#define TEST_MODE_0444 (SKID_MODE_OWNER_R | SKID_MODE_GROUP_R | SKID_MODE_OTHER_R)
#define TEST_MODE_0644 (SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | \
					    SKID_MODE_GROUP_R | \
                        SKID_MODE_OTHER_R)
#define TEST_MODE_0764 (SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | SKID_MODE_OWNER_X | \
                        SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | \
                        SKID_MODE_OTHER_R)
#define TEST_MODE_0775 (TEST_MODE_0444 | TEST_MODE_0220 | TEST_MODE_0111)
#define TEST_MODE_0777 (TEST_MODE_0444 | TEST_MODE_0222 | TEST_MODE_0111)
#define TEST_MODE_7000 (SKID_MODE_SET_UID | SKID_MODE_SET_GID | SKID_MODE_STICKYB)
#define TEST_MODE_7764 (TEST_MODE_7000 | TEST_MODE_0764)
#define TEST_MODE_7777 (TEST_MODE_7000 | TEST_MODE_0777)

/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

/*
 *	Determine the expected mode based on the owning process' umask.
 */
mode_t determine_exp_mode(mode_t mode_input);

/*
 *	Create a unique directory name, created from a timestamp and filename, resolved to
 *	WORKING_DIR to use for a test case.  Use free_devops_mem() to free the return value.
 */
char *get_test_dir_name(void);

/*
 *	Resolve paththame to the working directory in a standardized way.  Use free_devops_mem() to
 *	free the return value.  The base argument is optional and WORKING_DIR will be used by default.
 */
char *resolve_test_input(const char *pathname, const char *base);

/*
 *	Make the function call, check the expected return value, validate the results, and cleanup
 *	(if necessary).
 */
void run_test_case(char *dir_input, mode_t mode_input, int exp_return, bool cleanup);


mode_t determine_exp_mode(mode_t mode_input)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;  // Store errno values
	mode_t result = 0;        // Expected mode
	mode_t curr_umask = 0;    // The umask of the owning process

	// DETERMINE IT
	// Get the umask
	curr_umask = get_shell_umask(&errnum);
	ck_assert_msg(0 == errnum, "get_shell_umask() failed with [%d] %s\n",
		          errnum, strerror(errnum));
	// Determine the expected mode
	result = mode_input & ~curr_umask & 01777;  // See mkdir(2) + sticky bit

	// DONE
	return result;
}


char *get_test_dir_name(void)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;               // Errno values
	time_t now = 0;                        // The date in epoch seconds
	long nsecs = 0;                        // Current nanoseconds
	char *unique_dir = NULL;               // Unique directory name
	char dir_name[PATH_MAX + 2] = { 0 };   // Generated directory name

	// GET IT
	// Get times
	now = get_shell_time_now(&errnum);
	ck_assert_msg(0 == errnum, "get_shell_time_now() failed with [%d] %s\n",
		          errnum, strerror(errnum));
	nsecs = get_shell_nsec_now(&errnum);
	ck_assert_msg(0 == errnum, "get_shell_nsecs_now() failed with [%d] %s\n",
		          errnum, strerror(errnum));
	// Format timestamp
	errnum = format_time_terse(dir_name, PATH_MAX * sizeof(char), now);
	ck_assert_msg(0 == errnum, "format_time_terse() failed with [%d] %s\n",
		          errnum, strerror(errnum));
	dir_name[strlen(dir_name)] = '_';
	sprintf(dir_name + strlen(dir_name), "%ld", nsecs);  // Append the nanoseconds
	// Build dirname
	dir_name[strlen(dir_name)] = '-';  // Separate the timestamp and BASE_DIR_NAME
	strncat(dir_name, BASE_DIR_NAME, PATH_MAX - strlen(dir_name));
	// Resolve to WORKING_DIR
	unique_dir = resolve_test_input(dir_name, NULL);

	// DONE
	return unique_dir;
}


char *resolve_test_input(const char *pathname, const char *base)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;       // Errno values
	const char *base_name = base;  // Name of the repo
	char *resolved_name = NULL;    // pathname resolved to repo_name

	// SETUP
	if (NULL == base_name || 0x0 == *base_name)
	{
		base_name = WORKING_DIR;
	}

	// RESOLVE IT
	resolved_name = join_dir_to_path(base_name, pathname, false, &errnum);
	ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s\n", base_name,
				  pathname, errnum, strerror(errnum));
	ck_assert_msg(NULL != resolved_name, "resolve_to_repo(%s, %s) failed to resolve the path\n",
				  base_name, pathname);

	// DONE
	if (0 != errnum && resolved_name)
	{
		free_devops_mem((void **)&resolved_name);  // Best effort
	}
	return resolved_name;
}


void run_test_case(char *dir_input, mode_t mode_input, int exp_return, bool cleanup)
{
	// LOCAL VARIABLES
	int actual_ret = 0;                                // Return value of the tested function
	int errnum = 0;                                    // Catch errno values here
	mode_t exp_mode = determine_exp_mode(mode_input);  // Expected mode
	mode_t actual_mode = 0;                            // Actual dir_input mode, if it was created
	char *short_dir_input = dir_input;                 // Try to restrict long Check unit test msgs

	// SETUP
	if (short_dir_input && strlen(short_dir_input) >= DEFAULT_MAX_MSG_SIZE)
	{
		short_dir_input = "<LONG DIR INPUT>";  // Try to keep it short
	}

	// PRE-CLEANUP
	if (true == cleanup && true == is_directory(dir_input, &errnum))
	{
		errnum = remove_shell_dir(dir_input);
		ck_assert_msg(0 == errnum, "The first remove_shell_dir(%s) call errored with [%d] '%s'\n",
					  short_dir_input, errnum, strerror(errnum));
	}

	// RUN IT
	// Call the function
	actual_ret = create_dir(dir_input, mode_input);
	// Compare actual results to expected results
	ck_assert_msg(exp_return == actual_ret, "create_dir(%s, %o) returned [%d] '%s' "
				  "instead of [%d] '%s'\n", short_dir_input, mode_input, actual_ret,
				  strerror(actual_ret), exp_return, strerror(exp_return));
	// No need to check the results unless the test was expected to succeed
	if (0 == exp_return)
	{
		// 1. Does it exist?
		ck_assert_msg(true == is_path_there(dir_input), "Unable to locate '%s'\n", short_dir_input);
		// 2. Is it a directory?
		ck_assert_msg(true == is_directory(dir_input, &errnum), "'%s' is *not* a directory\n",
			          short_dir_input);
		ck_assert_msg(0 == errnum, "The is_directory(%s) call errored with [%d] '%s'\n",
					  short_dir_input, errnum, strerror(errnum));
		// 3. Verify permissions
		actual_mode = get_shell_file_perms(dir_input, &errnum);
		ck_assert_msg(0 == errnum, "The get_shell_file_perms(%s) call errored with [%d] '%s'\n",
					  short_dir_input, errnum, strerror(errnum));
		// Compare actual permissions to the mode_input (while ignoring unused test input bits)
		ck_assert_msg(actual_mode == exp_mode,
			          "create_dir(%s, %o) failed to set the mode to %o (saw %o instead)\n",
					  short_dir_input, mode_input, exp_mode, actual_mode);
	}

	// POST-CLEANUP
	if (true == cleanup)
	{
		errnum = remove_shell_dir(dir_input);
		ck_assert_msg(0 == errnum, "The second remove_shell_dir(%s) call errored with [%d] '%s'\n",
					  short_dir_input, errnum, strerror(errnum));
	}

	// DONE
	return;
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_abs_dir)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


// Enable this test case once *any* library is capable of deleting the directory this test created.
START_TEST(test_n02_rel_dir)
{
	// LOCAL VARIABLES
	int exp_return = 0;   // Expected return value for this test case
	bool cleanup = true;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();  // MAKE THIS A RELATIVE DIRECTORY
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	// Delete the test directory
	exp_return = delete_dir(input_abs_path);
	ck_assert_msg(0 == exp_return, "The delete_dir(%s) call errored with [%d] '%s'\n",
				  input_abs_path, exp_return, strerror(exp_return));
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


// Enable this test case once *any* library is capable of deleting the directory this test created.
START_TEST(test_n03_just_a_dir)
{
	// LOCAL VARIABLES
	int exp_return = 0;   // Expected return value for this test case
	bool cleanup = true;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();  // MAKE THIS JUST A DIRECTORY NAME
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	// Delete the test directory
	exp_return = delete_dir(input_abs_path);
	ck_assert_msg(0 == exp_return, "The delete_dir(%s) call errored with [%d] '%s'\n",
				  input_abs_path, exp_return, strerror(exp_return));
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_dirname)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	bool cleanup = false;     // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = NULL;
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


START_TEST(test_e02_empty_dirname)
{
	// LOCAL VARIABLES
	int exp_return = EINVAL;  // Expected return value for this test case
	bool cleanup = false;     // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = "\0 EMPTY STRING";
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


START_TEST(test_e03_missing_dir)
{
	// LOCAL VARIABLES
	int exp_return = ENOENT;  // Expected return value for this test case
	bool cleanup = false;     // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = "/not/found/here";
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


START_TEST(test_e04_dir_already_exists)
{
	// LOCAL VARIABLES
	int exp_return = EEXIST;  // Expected return value for this test case
	bool cleanup = false;     // Remove directory after test case has run
	int errnum = 0;           // Store errno values
	// Relative test case input: directory name
	char input_rel_path[] = { "code/test/test_input/" };
	// Absolute test case input: directory name
	char *input_abs_path = resolve_to_repo(SKID_REPO_NAME, input_rel_path, true, &errnum);
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// VALIDATE
	ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s\n",
		          SKID_REPO_NAME, input_rel_path, errnum, strerror(errnum));

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


/**************************************************************************************************/
/************************************** BOUNDARY TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_b01_smallest_mode)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = 0;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_b02_largest_mode)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = TEST_MODE_7777;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_b03_shortest_dir_name)
{
	// LOCAL VARIABLES
	int exp_return = 0;   // Expected return value for this test case
	bool cleanup = true;  // Remove directory after test case has run
	// Test case input: directory name
	char input_abs_path[] = { "/tmp/a" };
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


START_TEST(test_b04_longest_dir_name)
{
	// LOCAL VARIABLES
	int exp_return = 0;   // Expected return value for this test case
	bool cleanup = true;  // Remove directory after test case has run
	// Test case input: directory name
	char input_abs_path[PATH_MAX + 1] = { "/tmp/" };
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// SETUP
	for (int i = strlen(input_abs_path); i < PATH_MAX; i++)
	{
		input_abs_path[i] = 'b';
	}

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


START_TEST(test_b05_dir_name_too_long)
{
	// LOCAL VARIABLES
	int exp_return = ENAMETOOLONG;  // Expected return value for this test case
	bool cleanup = false;           // Remove directory after test case has run
	// Test case input: directory name
	char input_abs_path[(PATH_MAX * 2) + 1] = { "/tmp/" };
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// SETUP
	for (int i = strlen(input_abs_path); i < (PATH_MAX * 2); i++)
	{
		input_abs_path[i] = 'c';
	}

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


// Observed behavior is that, in invalid mode values, "don't care about those" bits are ignored.
START_TEST(test_s01_all_flags_enabled)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = 0xFF;

	// SETUP
	for (int i = 1; i < sizeof(mode_t); i++)
	{
		input_mode <<= 8;  // Make room for the next byte
		input_mode |= 0xFF;  // Turn on one byte's worth of bits
	}

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_s02_path_contains_non_dir)
{
	// LOCAL VARIABLES
	int exp_return = ENOTDIR;  // Expected return value for this test case
	bool cleanup = false;      // Remove directory after test case has run
	int errnum = 0;            // Store errno values
	// Absolute path to the repo directory
	char *repo_dir = resolve_to_repo(SKID_REPO_NAME, NULL, true, &errnum);
	// Relative path of the base test case input
	char input_rel_base[] = { "code/test/test_input/regular_file.txt" };
	// Absolute path of the base test case input
	char *input_abs_base = resolve_test_input(input_rel_base, repo_dir);
	// Test case input: directory name
	char input_abs_path[PATH_MAX + 2] = { 0 };
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// VALIDATE
	ck_assert_msg(0 == errnum, "resolve_to_repo(%s, NULL) failed with [%d] %s\n",
		          SKID_REPO_NAME, errnum, strerror(errnum));

	// SETUP
	strncpy(input_abs_path, input_abs_base, PATH_MAX);
	strncat(input_abs_path, "/special02", PATH_MAX + 1 - strlen(input_abs_path));

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&repo_dir);
	free_devops_mem((void **)&input_abs_base);
}
END_TEST


START_TEST(test_s03_special_bits)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = TEST_MODE_7764;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_s04_just_the_special_bits)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = TEST_MODE_7000;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


// Observed behavior is that, in invalid mode values, "don't care about those" bits are ignored.
START_TEST(test_s05_bit_field_ignores_invalid_bits)
{
	// LOCAL VARIABLES
	int exp_return = 0;    // Expected return value for this test case
	bool cleanup = false;  // Remove directory after test case has run
	// Test case input: directory name
	char *input_abs_path = get_test_dir_name();
	// Test case input: mode
	mode_t input_mode = 077777;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_s06_dirname_in_filename_format)
{
	// LOCAL VARIABLES
	int exp_return = 0;   // Expected return value for this test case
	bool cleanup = true;  // Remove directory after test case has run
	// Relative path of the test case input
	char input_abs_path[] = { "/tmp/regular_file.txt" };
	// Test case input: mode
	mode_t input_mode = TEST_MODE_0775;

	// RUN TEST
	run_test_case(input_abs_path, input_mode, exp_return, cleanup);
}
END_TEST


Suite *create_dir_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SDO_Create_Dir");  // Test suite
	TCase *tc_normal = tcase_create("Normal");      // Normal test cases
	TCase *tc_error = tcase_create("Error");        // Error test cases
	TCase *tc_boundary = tcase_create("Boundary");  // Error test cases
	TCase *tc_special = tcase_create("Special");    // Special test cases

	// SETUP TEST CASES
	tcase_add_test(tc_normal, test_n01_abs_dir);
	tcase_add_test(tc_normal, test_n02_rel_dir);
	tcase_add_test(tc_normal, test_n03_just_a_dir);
	tcase_add_test(tc_error, test_e01_null_dirname);
	tcase_add_test(tc_error, test_e02_empty_dirname);
	tcase_add_test(tc_error, test_e03_missing_dir);
	tcase_add_test(tc_error, test_e04_dir_already_exists);
	tcase_add_test(tc_boundary, test_b01_smallest_mode);
	tcase_add_test(tc_boundary, test_b02_largest_mode);
	tcase_add_test(tc_boundary, test_b03_shortest_dir_name);
	tcase_add_test(tc_boundary, test_b04_longest_dir_name);
	tcase_add_test(tc_boundary, test_b05_dir_name_too_long);
	tcase_add_test(tc_special, test_s01_all_flags_enabled);
	tcase_add_test(tc_special, test_s02_path_contains_non_dir);
	tcase_add_test(tc_special, test_s03_special_bits);
	tcase_add_test(tc_special, test_s04_just_the_special_bits);
	tcase_add_test(tc_special, test_s05_bit_field_ignores_invalid_bits);
	tcase_add_test(tc_special, test_s06_dirname_in_filename_format);
	suite_add_tcase(suite, tc_normal);
	suite_add_tcase(suite, tc_error);
	suite_add_tcase(suite, tc_boundary);
	suite_add_tcase(suite, tc_special);

	// DONE
	return suite;
}


int main(void)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno from the function call
	// Relative path for this test case's input
	char log_rel_path[] = { "./code/test/test_output/check_sdo_create_dir.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = create_dir_suite();
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
