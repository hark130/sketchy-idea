/*
 *  Check unit test suit for skid_dir_operations.h's read_dir_contents() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sdo_read_dir_contents.bin && \
code/dist/check_sdo_read_dir_contents.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sdo_read_dir_contents.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sdo_read_dir_contents.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sdo_read_dir_contents.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sdo_read_dir_contents.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sdo_read_dir_contents.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>						// START_TEST(), END_TEST
#include <stdlib.h>						// EXIT_FAILURE, EXIT_SUCCESS
#include <linux/limits.h>				// NAME_MAX, PATH_MAX
#include <stdio.h>						// sprintf()
// Local includes
#include "devops_code.h"				// resolve_to_repo(), SKID_REPO_NAME
#include "skid_dir_operations.h"		// read_dir_contents()
#include "skid_file_metadata_read.h"	// is_directory()
#include "skid_macros.h"				// SKID_MODE_* macros

#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
#define WORKING_DIR "/tmp"  // The working directory for these test cases
#define BASE_DIR_NAME "read_dir_contents"  // Incorporate this into the auto-generated dir names

/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

/*
 *	Check the read_dir_contents() return value against test case input and the environment.
 *	Do not call this method for a test that was expected to fail.  This function does not validate
 *	input.
 */
void check_return_value(char *dir_input, char **return_arr, size_t return_cap, char **artifact_arr,
	                    bool recurse, bool create, unsigned int num_files, unsigned int tree_width);

/*
 *	Replicate the behavior of read_dir_contents() using a combination of devops_code's
 *	remove_a_file() and remove_shell_dir().  Works through the array in reverse order.
 */
int destroy_shell_tree(char **old_path_tree);

/*
 *	Counts the number of non-NULL entries in return_arr.  Stops counting when the first NULL is
 *	encountered.  This function does not validate input.
 */
size_t determine_arr_len(char **return_arr);

/*
 *	Create a unique directory name, created from a timestamp and filename, resolved to
 *	WORKING_DIR to use for a test case.  Use free_devops_mem() to free the return value.
 */
char *get_test_dir_name(void);

/*
 *	Verify an exact match of needle is found in haystack.  Also returns false for bad input.
 */
bool is_needle_in_haystack(char *needle, char **haystack);

/*
 *	Resolve paththame to the working directory in a standardized way.  Use free_devops_mem() to
 *	free the return value.  The base argument is optional and WORKING_DIR will be used by default.
 */
char *resolve_test_input(const char *pathname, const char *base);

/*
 *	Make the function call, check the expected return value, and validate the results.
 *	If create is true, this function will create dir_input, using devops_code's create_path_tree(),
 *	before calling read_dir_contents(), and attempt to delete them after the test.
 */
void run_test_case(char *dir_input, bool recurse, int exp_result, bool create,
	               unsigned int num_files, unsigned int tree_width, unsigned int tree_depth);


void check_return_value(char *dir_input, char **return_arr, size_t return_cap, char **artifact_arr,
	                    bool recurse, bool create, unsigned int num_files, unsigned int tree_width)
{
	// LOCAL VARIABLES
	size_t return_len = 0;    // Length of the return_arr
	size_t artifact_len = 0;  // Length of the artifact_arr

	// CHECK IT
	// Use Cases:
	//	Something was created
	//		All
	//			verify everything is in dir_input (strstr)
	//			verify all return arr entries are found in artifact_arr
	//			verify all return arr entries exist
	//			verify counted entries do no exceed returned capacity
	//		recurse
	//			verify length of return_arr and artifact_arr are the same
	//		no recurse
	//			verify length == num_files + tree_width
	//	Nothing was created
	//		If not-NULL
	//			verify everything is in dir_input (strstr)
	//			verify everything exists
	//		If NULL
	//			let it go
	// fprintf(stderr, "MADE IT INTO %s\n", __func__);  // DEBUGGING
	if (true == create)
	{
		// Measure both arrays
		return_len = determine_arr_len(return_arr);
		artifact_len = determine_arr_len(artifact_arr) - 1;  // Less one for the top-level dir
		// fprintf(stderr, "RETURNED ARRAY IS %zu LONG\n", return_len);  // DEBUGGING
		// fprintf(stderr, "ARTIFACT ARRAY IS %zu LONG\n", artifact_len);  // DEBUGGING

		// Verify length
		if (true == recurse)
		{
			// if (return_len != artifact_len)
			// {
			// 	// fprintf(stderr, "EXPECTED %zu BUT MEASURED %zu\n", artifact_len, return_len);  // DEBUGGING
			// 	// fprintf(stderr, "ARTIFACT STRINGS:\n");
			// 	for (int i = 1; i < artifact_len + 1; i++)
			// 	{
			// 		fprintf(stderr, "\t'%s'\n", artifact_arr[i]);  // DEBUGGING
			// 	}
			// 	// fprintf(stderr, "RETURNED STRINGS:\n");
			// 	for (int i = 0; i < return_len; i++)
			// 	{
			// 		fprintf(stderr, "\t'%s'\n", return_arr[i]);  // DEBUGGING
			// 	}
			// }
			ck_assert_msg(return_len == artifact_len, "Recursive content array size mismatch: "
				          "return array for '%s' is %zu long but the artifact array is %zu long\n",
				          dir_input, return_len, artifact_len);
		}
		else
		{
			// if (return_len != (num_files + tree_width))
			// {
			// 	fprintf(stderr, "EXPECTED %u BUT MEASURED %zu\n", num_files + tree_width, return_len);  // DEBUGGING
			// 	fprintf(stderr, "ARTIFACT STRINGS:\n");
			// 	for (int i = 1; i < artifact_len + 1; i++)
			// 	{
			// 		fprintf(stderr, "\t'%s'\n", artifact_arr[i]);  // DEBUGGING
			// 	}
			// 	fprintf(stderr, "RETURNED STRINGS:\n");
			// 	for (int i = 0; i < return_len; i++)
			// 	{
			// 		fprintf(stderr, "\t'%s'\n", return_arr[i]);  // DEBUGGING
			// 	}
			// }
			ck_assert_msg(return_len == (num_files + tree_width),
				          "Detected non-recursive mismatch in the length of the returned array. "
				          "Expected %u but counted %zu in the returned array\n",
				          (num_files + tree_width), return_len);
		}
		ck_assert_msg(return_len <= return_cap,
			          "The capacity of the return value (%zu) is too small to hold %zu entries",
			          return_cap, return_len);
		// Verify content
		for (size_t i = 0; i < return_len; i++)
		{
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
			// Verify a found path was in dir_input
			ck_assert_msg(strstr(return_arr[i], dir_input) == return_arr[i],
				          "Found an entry '%s' that is not in the original dir_input ('%s')\n",
				          return_arr[i], dir_input);
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
			// Verify a found path was a created path
			ck_assert_msg(true == is_needle_in_haystack(return_arr[i], artifact_arr),
				          "Unable to find '%s' in the framework's test case input\n",
				          return_arr[i]);
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
			// Verify the existence of a found path
			ck_assert_msg(true == is_path_there(return_arr[i]),
				          "An entry was found ('%s') that does not exist\n", return_arr[i]);
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
		}
	}
	else
	{
		if (return_arr && *return_arr)
		{
			ck_assert_msg(return_cap > 0, "Invalid capacity size of %zu for the return value\n",
				          return_cap);
			for (size_t i = 0; i < return_cap; i++)
			{
				if (NULL == return_arr[i])
				{
					break;  // Reached the end
				}
				ck_assert_msg(strstr(return_arr[i], dir_input) == return_arr[i],
					          "Found an entry '%s' that is not in the original dir_input ('%s')\n",
					          return_arr[i], dir_input);
				ck_assert_msg(true == is_path_there(return_arr[i]),
					          "An entry was found ('%s') that does not exist\n", return_arr[i]);
			}
		}
	}

	// DONE
	return;
}


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
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
			tmp_path = old_path_tree[i];
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
			// Exists?
			// fprintf(stderr, "ABOUT TO LOOK FOR: %s\n", tmp_path);  // DEBUGGING
			if (true == is_path_there(tmp_path))
			{
				// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
				// File?
				if (true == is_regular_file(tmp_path, &result))
				{
					// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
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
				// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
				if (true == is_directory(tmp_path, &result))
				{
					// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
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
			// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
		}
	}

	// DONE
	return result;
}


size_t determine_arr_len(char **return_arr)
{
	// LOCAL VARIABLES
	size_t return_len = 0;        // Number of entries in return_arr
	char **tmp_arr = return_arr;  // Iterating variable

	// INPUT VALIDATION
	if (tmp_arr)
	{
		while (*tmp_arr)
		{
			return_len++;
			tmp_arr++;  // Next index
		}
	}

	// DONE
	return return_len;
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


bool is_needle_in_haystack(char *needle, char **haystack)
{
	// LOCAL VARIABLES
	bool found_it = false;        // Found needle in haystack
	char **tmp_stack = haystack;  // Iterating variable
	char *tmp_str = NULL;         // Temp string pointer

	// INPUT VALIDATION
	if (needle && *needle && haystack && *haystack && *(*haystack))
	{
		while (*tmp_stack)
		{
			tmp_str = *tmp_stack;
			// fprintf(stderr, "Looking for '%s' in '%s'\n", needle, tmp_str);  // DEBUGGING
			if (!strcmp(tmp_str, needle))
			{
				// fprintf(stderr, "Found '%s' in '%s'\n", needle, tmp_str);  // DEBUGGING
				found_it = true;  // Found it!
				break;  // Stop looking
			}
			else if('/' == tmp_str[strlen(tmp_str)-1] && strlen(tmp_str) == (strlen(needle) + 1) \
				    && !strncmp(tmp_str, needle, strlen(needle)))
			{
				// The devops code that creates the hiearchy appends all directories with trailing
				// delimiters but the production code does not.
				// fprintf(stderr, "Found special case '%s' in '%s'\n", needle, tmp_str);  // DEBUGGING
				found_it = true;  // Found it...
				break;  // Stop looking
			}
			else
			{
				tmp_stack++;  // Keep looking
			}
		}
	}

	// DONE
	return found_it;
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


void run_test_case(char *dir_input, bool recurse, int exp_result, bool create,
	               unsigned int num_files, unsigned int tree_width, unsigned int tree_depth)
{
	// LOCAL VARIABLES
	int act_result = CANARY_INT;  // Return value of the tested function
	size_t capacity = 0;          // Capacity of read_dir_contents() return value
	int errnum = 0;               // Catch errno values here
	char **artifact_arr = NULL;   // Paths created by create_path_tree()
	char **content_arr = NULL;    // Return value from read_dir_contents()

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
	content_arr = read_dir_contents(dir_input, recurse, &act_result, &capacity);
	// fprintf(stderr, "MADE IT HERE\n");  // DEBUGGING
	// Compare actual results to expected results
	ck_assert_msg(exp_result == act_result, "read_dir_contents(%s) resulted in [%d] '%s' "
				  "instead of [%d] '%s'\n", dir_input, act_result, strerror(act_result),
				  exp_result, strerror(exp_result));
	// No need to check the results unless the test was expected to succeed
	if (0 == exp_result)
	{
		// Tree width doesn't matter if there's no depth (and vice versa)
		check_return_value(dir_input, content_arr, capacity, artifact_arr, recurse,
			               create, num_files, (tree_depth > 0) ? tree_width : 0);
	}
	else
	{
		ck_assert_msg(NULL == content_arr, "An expected failure resulted in found paths\n");
	}

	// POST-TEST CLEANUP
	// Delete test artifacts
	if (true == create)
	{
		// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
		errnum = destroy_shell_tree(artifact_arr);
		ck_assert_msg(0 == errnum, "The destroy_shell_tree(%p) call failed with [%d] '%s'\n",
					  artifact_arr, errnum, strerror(errnum));
		// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
	}
	// Free path tree
	if (artifact_arr)
	{
		// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
		free_path_tree(&artifact_arr);  // Best effort
		// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
	}
	// Free return value
	if (content_arr)
	{
		// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
		errnum = free_skid_dir_contents(&content_arr);
		ck_assert_msg(0 == errnum, "The library failed to free the return value with [%d] '%s'\n",
			          errnum, strerror(errnum));
		// fprintf(stderr, "MADE IT TO %d\n", __LINE__);  // DEBUGGING
	}

	// DONE
	return;
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_no_recurse_abs_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n02_no_recurse_rel_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "./check_sdo_read_dir_contents_normal_02" };
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_n03_no_recurse_just_a_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "check_sdo_read_dir_contents_normal_03" };
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_n04_no_recurse_abs_dir_no_files_one_subdir_one_level)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 1;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n05_no_recurse_abs_dir_no_files_a_few_subdirs_one_level)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n06_no_recurse_abs_dir_one_file_no_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 1;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n07_no_recurse_abs_dir_a_few_files_no_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n08_no_recurse_abs_dir_a_few_files_a_few_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n09_no_recurse_abs_dir_a_couple_files_a_couple_subdirs_a_few_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 2;   // Number of files to add to the hierarchy
	unsigned int tree_width = 2;  // Number of sub-directories at each level
	unsigned int tree_depth = 3;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n10_recurse_abs_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n11_recurse_rel_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "./check_sdo_read_dir_contents_normal_02" };
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_n12_recurse_just_a_dir_no_tree)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[] = { "check_sdo_read_dir_contents_normal_03" };
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_n13_recurse_abs_dir_no_files_one_subdir_one_level)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 1;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n14_recurse_abs_dir_no_files_a_few_subdirs_one_level)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n15_recurse_abs_dir_one_file_no_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 1;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n16_recurse_abs_dir_a_few_files_no_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n17_recurse_abs_dir_a_few_files_a_few_subdirs_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


START_TEST(test_n18_recurse_abs_dir_a_couple_files_a_couple_subdirs_a_few_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = true;          // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 2;   // Number of files to add to the hierarchy
	unsigned int tree_width = 2;  // Number of sub-directories at each level
	unsigned int tree_depth = 3;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

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
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = EINVAL;      // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_e02_empty_dirname)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = "\0 EMPTY STRING";
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = EINVAL;      // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_e03_missing_dir)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = "/not/found/here";
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = ENOENT;      // Expected return value for this test case
	bool create = false;          // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
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
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 0;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_b02_longest_dir_name)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char input_test_path[PATH_MAX + 1] = { "/tmp/" };
	// Base length of input_test_path
	size_t abs_path_len = strlen(input_test_path);
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
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
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
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
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 1;   // Number of files to add to the hierarchy
	unsigned int tree_width = 1;  // Number of sub-directories at each level
	unsigned int tree_depth = 1;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);
}
END_TEST


START_TEST(test_s02_not_a_dir)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;      // Errno values
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = ENOTDIR;     // Expected return value for this test case
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
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


// No sub-dirs will be created because there's no depth
START_TEST(test_s03_no_recurse_abs_dir_a_few_files_a_few_subdirs_but_no_levels)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 3;  // Number of sub-directories at each level
	unsigned int tree_depth = 0;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


// No sub-dirs will be created because there's no tree width
START_TEST(test_s04_no_recurse_abs_dir_a_few_files_a_few_levels_but_no_subdirs)
{
	// LOCAL VARIABLES
	// Test case input: directory name
	char *input_test_path = get_test_dir_name();
	bool recurse = false;         // Test case input: controls recursion
	int exp_result = 0;           // Expected return value for this test case
	bool create = true;           // Create dir prior to test case (and cleanup if necessary)
	unsigned int num_files = 3;   // Number of files to add to the hierarchy
	unsigned int tree_width = 0;  // Number of sub-directories at each level
	unsigned int tree_depth = 3;  // Number of sub-directory levels

	// RUN TEST
	run_test_case(input_test_path, recurse, exp_result, create, num_files, tree_width, tree_depth);

	// CLEANUP
	free_devops_mem((void **)&input_test_path);
}
END_TEST


Suite *read_dir_contents_suite(void)
{
	// LOCAL VARIABLES
	// Test Suite
	Suite *suite = suite_create("SDO_Read_Dir_Contents");
	TCase *tc_normal = tcase_create("Normal");      // Normal test cases
	TCase *tc_error = tcase_create("Error");        // Error test cases
	TCase *tc_boundary = tcase_create("Boundary");  // Error test cases
	TCase *tc_special = tcase_create("Special");    // Special test cases

	// SETUP TEST CASES
	tcase_add_test(tc_normal, test_n01_no_recurse_abs_dir_no_tree);
	tcase_add_test(tc_normal, test_n02_no_recurse_rel_dir_no_tree);
	tcase_add_test(tc_normal, test_n03_no_recurse_just_a_dir_no_tree);
	tcase_add_test(tc_normal, test_n04_no_recurse_abs_dir_no_files_one_subdir_one_level);
	tcase_add_test(tc_normal, test_n05_no_recurse_abs_dir_no_files_a_few_subdirs_one_level);
	tcase_add_test(tc_normal, test_n06_no_recurse_abs_dir_one_file_no_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n07_no_recurse_abs_dir_a_few_files_no_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n08_no_recurse_abs_dir_a_few_files_a_few_subdirs_no_levels);
	tcase_add_test(tc_normal,
		           test_n09_no_recurse_abs_dir_a_couple_files_a_couple_subdirs_a_few_levels);
	tcase_add_test(tc_normal, test_n10_recurse_abs_dir_no_tree);
	tcase_add_test(tc_normal, test_n11_recurse_rel_dir_no_tree);
	tcase_add_test(tc_normal, test_n12_recurse_just_a_dir_no_tree);
	tcase_add_test(tc_normal, test_n13_recurse_abs_dir_no_files_one_subdir_one_level);
	tcase_add_test(tc_normal, test_n14_recurse_abs_dir_no_files_a_few_subdirs_one_level);
	tcase_add_test(tc_normal, test_n15_recurse_abs_dir_one_file_no_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n16_recurse_abs_dir_a_few_files_no_subdirs_no_levels);
	tcase_add_test(tc_normal, test_n17_recurse_abs_dir_a_few_files_a_few_subdirs_no_levels);
	tcase_add_test(tc_normal,
		           test_n18_recurse_abs_dir_a_couple_files_a_couple_subdirs_a_few_levels);
	tcase_add_test(tc_error, test_e01_null_dirname);
	tcase_add_test(tc_error, test_e02_empty_dirname);
	tcase_add_test(tc_error, test_e03_missing_dir);
	tcase_add_test(tc_boundary, test_b01_shortest_dir_name);
	tcase_add_test(tc_boundary, test_b02_longest_dir_name);
	tcase_add_test(tc_special, test_s01_dirname_in_filename_format);
	tcase_add_test(tc_special, test_s02_not_a_dir);
	tcase_add_test(tc_special, test_s03_no_recurse_abs_dir_a_few_files_a_few_subdirs_but_no_levels);
	tcase_add_test(tc_special, test_s04_no_recurse_abs_dir_a_few_files_a_few_levels_but_no_subdirs);
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
	char log_rel_path[] = { "./code/test/test_output/check_sdo_read_dir_contents.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = read_dir_contents_suite();
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
