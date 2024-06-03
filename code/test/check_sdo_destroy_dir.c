/*
 *  Check unit test suit for skid_dir_operations.h's destroy_dir() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sdo_destroy_dir.bin && \
code/dist/check_sdo_destroy_dir.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sdo_destroy_dir.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sdo_destroy_dir.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sdo_destroy_dir.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sdo_destroy_dir.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sdo_destroy_dir.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>						// START_TEST(), END_TEST
#include <stdlib.h>						// EXIT_FAILURE, EXIT_SUCCESS
#include <linux/limits.h>				// NAME_MAX, PATH_MAX
#include <stdio.h>						// sprintf()
// Local includes
#include "devops_code.h"				// resolve_to_repo(), SKID_REPO_NAME
#include "skid_dir_operations.h"		// destroy_dir()
#include "skid_file_metadata_read.h"	// is_directory()
#include "skid_macros.h"				// SKID_MODE_* macros

#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
#define WORKING_DIR "/tmp"  // The working directory for these test cases
#define BASE_DIR_NAME "destroy_dir"  // Incorporate this into the auto-generated directory names

/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

/*
 *	Replicate the behavior of destroy_dir() using a combination of devops_code's remove_a_file()
 *	and remove_shell_dir().  Works through the array in reverse order.
 */
int destroy_shell_tree(char **old_path_tree);

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
 *	Make the function call, check the expected return value, and validate the results.
 *	If create is true, this function will create dir_input, using devops_code's create_path_tree(),
 *	before calling destroy_dir() and attempt to manually remove it all if destroy_dir() fails.
 */
void run_test_case(char *dir_input, int exp_return, bool create, unsigned int num_files,
	               unsigned int tree_width, unsigned int tree_depth);


int destroy_shell_tree(char **old_path_tree)
{
	// LOCAL VARIABLES
	int result = 0;                  // Errno values
	char **tmp_arr = old_path_tree;  // Iterating array pointer
	char *tmp_path = NULL;           // Temporary string pointer
	int str_count = 0;               // Number of strings in old_path_tree

	// INPUT VALIDATION
	if (NULL == old_path_tree || NULL == *old_path_tree)
	{
		result = EINVAL;  // Bad input
	}

	// DESTORY IT
	// Count the strings
	if (0 == result)
	{
		while (NULL != *tmp_arr && strlen(*tmp_arr) > 0)
		{
			str_count++;
			tmp_arr++;
		}
		if (0 == str_count)
		{
			result = EINVAL;  // The array only contained empty strings
		}
	}
	// Iterate the array in reverse order
	if (0 == result)
	{
		for (int i = str_count - 1; i >= 0; i--)
		{
			tmp_path = tmp_arr[i];
			// Exists?
			if (true == is_path_there(tmp_path))
			{
				// File?
				if (true == is_regular_file(tmp_path, &result))
				{
					result = remove_a_file(tmp_path, false);
					ck_assert_msg(0 == result, "remove_a_file(%s, false) failed with [%d] %s\n",
						          tmp_path, result, strerror(result));
					continue;  // File removed... keep going
				}
				else
				{
					ck_assert_msg(0 == result, "is_regular_file(%s) failed with [%d] %s\n",
						          tmp_path, result, strerror(result));
				}
				// Dir?
				if (true == is_directory(tmp_path, &result))
				{
					result = remove_shell_dir(tmp_path);
					ck_assert_msg(0 == result, "remove_shell_dir(%s) failed with [%d] %s\n",
						          tmp_path, result, strerror(result));
					continue;  // Directory removed... keep going
				}
				else
				{
					ck_assert_msg(0 == result, "is_directory(%s) failed with [%d] %s\n",
						          tmp_path, result, strerror(result));
				}
				// How did we get here?
				ck_abort_msg("'%s' exists but is not a file or directory?!", tmp_path);
			}
		}
	}

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
	ck_assert_msg(0 == errnum, "join_dir_to_path(%s, %s) failed with [%d] %s\n", base_name,
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


void run_test_case(char *dir_input, int exp_return, bool create, unsigned int num_files,
	               unsigned int tree_width, unsigned int tree_depth)
{
	// LOCAL VARIABLES
	int actual_ret = 0;                   // Return value of the tested function
	int errnum = 0;                       // Catch errno values here
	char **artifact_arr = NULL;           // Paths created by create_path_tree()

	// SETUP
	if (true == create)
	{
		artifact_arr = create_path_tree(dir_input, num_files, tree_width, tree_depth, &errnum);
		ck_assert_msg(0 == errnum, "create_path_tree(%s, %u, %u, %u) failed with [%d] '%s'\n",
			          dir_input, num_files, tree_width, tree_depth, errnum, strerror(errnum));
		ck_assert_msg(NULL != artifact_arr, "create_path_tree(%s, %u, %u, %u) failed silently\n",
					  dir_input, num_files, tree_width, tree_depth);
		ck_assert_msg(NULL != *artifact_arr, "create_path_tree(%s, %u, %u, %u)'s array is empty\n",
					  dir_input, num_files, tree_width, tree_depth);  // Should include dir_input
	}

	// RUN IT
	// Call the function
	actual_ret = destroy_dir(dir_input);
	// Compare actual results to expected results
	ck_assert_msg(exp_return == actual_ret, "destroy_dir(%s) returned [%d] '%s' "
				  "instead of [%d] '%s'\n", dir_input, actual_ret, strerror(actual_ret),
				  exp_return, strerror(exp_return));
	// No need to check the results unless the test was expected to succeed
	if (0 == exp_return)
	{
		// Does it exist?
		ck_assert_msg(false == is_path_there(dir_input), "Located '%s'\n", dir_input);
	}

	// POST-TEST CLEANUP
	// Delete test artifacts
	if (true == create)
	{
		errnum = destroy_shell_tree(artifact_arr);
		ck_assert_msg(0 == errnum, "The destroy_shell_tree(%p) call failed with [%d] '%s'\n",
					  artifact_arr, errnum, strerror(errnum));
	}
	// Free path tree
	if (artifact_arr)
	{
		free_path_tree(&artifact_arr);  // Best effort
	}

	// DONE
	return;
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_abs_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n02_rel_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "./check_sdo_destroy_dir_normal_02" };
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_n03_just_a_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "check_sdo_destroy_dir_normal_03" };
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_n04_abs_dir_no_files_one_subdir_one_level)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 1;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n05_abs_dir_no_files_a_few_subdirs_one_level)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n06_abs_dir_one_file_no_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 1;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n07_abs_dir_a_few_files_no_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n08_abs_dir_a_few_files_a_few_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n09_abs_dir_a_couple_files_a_couple_subdirs_a_few_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 2;   // Number of files to add to the hierarchy
	unsigned int tree_width = 2;  // Number of sub-directories at each level
	unsigned int tree_depth = 3;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_dirname)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = NULL;
	int exp_return = EINVAL;      // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_e02_empty_dirname)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = "\0 EMPTY STRING";
	int exp_return = EINVAL;      // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_e03_missing_dir)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = "/not/found/here";
	int exp_return = ENOENT;      // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


/**************************************************************************************************/
/************************************** BOUNDARY TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_b01_shortest_dir_name)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "/tmp/a" };
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_b02_longest_dir_name)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[PATH_MAX + 1] = { "/tmp/" };
	// Base length of input_test_path
	size_t abs_path_len = strlen(input_test_path);
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 1;   // Number of files to add to the hierarchy
	unsigned int tree_width = 1;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// SETUP
	for (int i = abs_path_len; i < PATH_MAX && i < (NAME_MAX + abs_path_len); i++)
	{
		input_test_path[i] = 'b';
	}

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_s01_dirname_in_filename_format)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "/tmp/not_a_file.txt" };
	int exp_return = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 1;   // Number of files to add to the hierarchy
	unsigned int tree_width = 1;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_s02_not_a_dir)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;      // Errno values
	int exp_return = ENOTDIR;     // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels
	// Relative test case input: directory name
	char input_rel_path[] = { "code/test/test_input/regular_file.txt/special02/" };
	// Test case input: directory name
	char *input_test_path = resolve_to_repo(SKID_REPO_NAME, input_rel_path, true, &errnum);

	// VALIDATE
	ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s, true) failed with [%d] %s\n",
		          SKID_REPO_NAME, input_rel_path, errnum, strerror(errnum));

	// RUN TEST
	run_test_case(input_test_path, exp_return, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


Suite *destroy_dir_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SDO_Destroy_Dir");  // Test suite
	TCase *tc_normal = tcase_create("Normal");      // Normal test cases
	TCase *tc_error = tcase_create("Error");        // Error test cases
	TCase *tc_boundary = tcase_create("Boundary");  // Error test cases
	TCase *tc_special = tcase_create("Special");    // Special test cases

	// SETUP TEST CASES
	tcase_add_test(tc_normal, test_n01_abs_dir_no_tree);
	tcase_add_test(tc_normal, test_n02_rel_dir_no_tree);
	tcase_add_test(tc_normal, test_n03_just_a_dir_no_tree);
	tcase_add_test(tc_normal, test_n04_abs_dir_no_files_one_subdir_one_level);
	tcase_add_test(tc_normal, test_n05_abs_dir_no_files_a_few_subdirs_one_level);
	tcase_add_test(tc_normal, test_n06_abs_dir_one_file_no_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n07_abs_dir_a_few_files_no_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n08_abs_dir_a_few_files_a_few_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n09_abs_dir_a_couple_files_a_couple_subdirs_a_few_levels);
	tcase_add_test(tc_error, test_e01_null_dirname);
	tcase_add_test(tc_error, test_e02_empty_dirname);
	tcase_add_test(tc_error, test_e03_missing_dir);
	tcase_add_test(tc_boundary, test_b01_shortest_dir_name);
	tcase_add_test(tc_boundary, test_b02_longest_dir_name);
	tcase_add_test(tc_special, test_s01_dirname_in_filename_format);
	tcase_add_test(tc_special, test_s02_not_a_dir);
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
	char log_rel_path[] = { "./code/test/test_output/check_sdo_destroy_dir.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = destroy_dir_suite();
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
