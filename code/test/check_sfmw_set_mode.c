/*
 *  Check unit test suit for skid_file_metadata_write.h's set_mode() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmw_set_mode.bin && \
code/dist/check_sfmw_set_mode.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmw_set_mode.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sfmw_set_mode.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sfmw_set_mode.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sfmw_set_mode.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sfmw_set_mode.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>						// START_TEST(), END_TEST
#include <stdlib.h>
// Local includes
#include "devops_code.h"				// resolve_to_repo(), SKID_REPO_NAME
#ifndef SKID_DEBUG
#define SKID_DEBUG
#endif  /* SKID_DEBUG */
#include "skid_debug.h"     			// FPRINTF_ERR()
#include "skid_file_metadata_read.h"	// get_group(), get_owner()
#include "skid_file_metadata_write.h"	// set_mode()

// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
// Common Mode Macros
#define TEST_MODE_0444 (SKID_MODE_OWNER_R | \
                        SKID_MODE_GROUP_R | \
                        SKID_MODE_OTHER_R)
#define TEST_MODE_0644 (SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | \
					    SKID_MODE_GROUP_R | \
                        SKID_MODE_OTHER_R)
#define TEST_MODE_0764 (SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | SKID_MODE_OWNER_X | \
                        SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | \
                        SKID_MODE_OTHER_R)
#define TEST_MODE_0777 (SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | SKID_MODE_OWNER_X | \
                        SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | SKID_MODE_GROUP_X | \
                        SKID_MODE_OTHER_R | SKID_MODE_OTHER_W | SKID_MODE_OTHER_X)
#define TEST_MODE_7000 (SKID_MODE_SET_UID | SKID_MODE_SET_GID | SKID_MODE_STICKYB)
#define TEST_MODE_7764 (TEST_MODE_7000 | TEST_MODE_0764)
#define TEST_MODE_7777 (TEST_MODE_7000 | TEST_MODE_0777)


/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

// This struct defines details about test files to aid in test execution and teardown() recovery.
typedef struct _cssmTestPathDetails
{
	char *pathname;    // Heap-allocated buffer containing the pathname resolved to the repo dir
	mode_t orig_mode;  // The original mode
} testPathDetails, *testPathDetails_ptr;

testPathDetails_ptr test_dir_details;  // Heap-allocated struct containing test directory details
testPathDetails_ptr test_file_details;  // Heap-allocated struct containing test filename details
testPathDetails_ptr test_pipe_details;  // Heap-allocated struct containing test pipe details
testPathDetails_ptr test_socket_details;  // Heap-allocated struct containing test socket details
testPathDetails_ptr test_symlink_details;  // Heap-allocated struct with test symbolic link details
testPathDetails_ptr test_bad_path_details;  // Heap-allocated struct with a bad path

/*
 *	Reset the permissions of the test_*_details globals back to their original mode values.
 */
void reset_global_modes(void);

/*
 *	Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *	the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *	Call set_mode(), get the new perms, and validate the results.  If pathname is a symbolic link,
 *	provide the sym_link's target as target_name.  If target_name is NULL, it will be ignored.
 */
void run_test_case(const char *pathname, const char *target_name, mode_t test_mode, int exp_return);

/*
 *	Resolve the named pipe and raw socket default filenames to the repo and store the heap
 *	memory pointer in the globals.
 */
void setup(void);

/*
 *	Allocate memory for a testPathDetails struct and populate it.  Ignores ENOENT errors.
 */
testPathDetails_ptr setup_struct(const char *pathname);

/*
 *	Update new_struct with the new_struct->pathname's current mode.  Does not ignore errors
 *	if must_succeed is true.  Always returns the error received.
 */
int setup_struct_mode(testPathDetails_ptr new_struct, bool must_succeed);

/*
 *	Delete the named pipe and raw socket files.  Then, free the heap memory arrays.
 */
void teardown(void);

/*
 *	Free all allocate memory in the testPathDetails struct in a gently tolerant way.
 */
void teardown_struct(testPathDetails_ptr* old_struct);


void reset_global_modes(void)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;  // Errno values
	// Array of the global pointers to iterate over
	testPathDetails_ptr test_structs[] = {test_dir_details, test_file_details, test_pipe_details,
										  test_socket_details, test_symlink_details};

	// RESET PERMISSIONS
	for (int i = 0; i < (sizeof(test_structs) / sizeof(test_structs[0])); i++)
	{
		if (test_structs[i])
		{
			// Reset mode
			errnum = set_shell_perms(test_structs[i]->pathname, test_structs[i]->orig_mode);
			if (EPERM == errnum)
			{
				FPRINTF_ERR("%s You may need to reset the owner of '%s' to %o\n",
					        DEBUG_WARNG_STR, test_structs[i]->pathname, test_structs[i]->orig_mode);
			}
			else
			{
				ck_assert_msg(0 == errnum, "set_shell_perms(%s, %o) failed with [%d] %s",
					          test_structs[i]->pathname, test_structs[i]->orig_mode,
					          errnum, strerror(errnum));
			}
		}
	}
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


void run_test_case(const char *pathname, const char *target_name, mode_t test_mode, int exp_return)
{
	// LOCAL VARIABLES
	int errnum = 0;                     // Errno values
	int actual_ret = 0;                 // Return value of the tested function
	mode_t new_mode = 0;                // Post-execution mode
	const char *check_mode = pathname;  // Filename to verify pre/post-test permissions

	// SETUP
	if (true == is_sym_link(pathname, &errnum) && target_name && *target_name)
	{
		check_mode = target_name;
	}

	// RUN IT
	actual_ret = set_mode(pathname, test_mode);

	// POST-TEST
	// Compare actual results to expected results
	ck_assert_msg(exp_return == actual_ret, "set_mode(%s, %o) returned [%d] '%s' "
				  "instead of [%d] '%s", pathname, test_mode, actual_ret, strerror(actual_ret),
				  exp_return, strerror(exp_return));
	// No need to compare new_mode against test_mode if the expected result is "error"
	if (0 == exp_return)
	{
		// Get the current permissions
		new_mode = get_shell_file_perms(check_mode, &errnum);
		ck_assert_msg(0 == errnum, "The second get_shell_file_perms(%s) errored with [%d] '%s'\n",
					  check_mode, errnum, strerror(errnum));
		// Compare actual permissions to the test_mode (while ignoring unused test input bits)
		ck_assert_msg(new_mode == (new_mode & test_mode),
			          "set_mode(%s, %o) failed to set the mode (saw %o instead)\n",
					  check_mode, test_mode, new_mode);
	}

	// DONE
	return;
}


void setup(void)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;                                             // Errno values
	char directory[] = { "./code/test/test_input/" };                    // Default input: dir
	char named_pipe[] = { "./code/test/test_input/named_pipe" };         // Default input: pipe
	char raw_socket[] = { "./code/test/test_input/raw_socket" };         // Default input: socket
	char reg_file[] = { "./code/test/test_input/regular_file.txt" };     // Default input: file
	char sym_link[] = { "./code/test/test_input/sym_link.txt" };         // Default input: symlink
	char bad_path[] = { "./code/test/test_input/named_pipe/bad_path" };  // Default input: bad path

	// SET IT UP
	// Allocate structs
	test_dir_details = setup_struct(directory);
	test_pipe_details = setup_struct(named_pipe);
	test_socket_details = setup_struct(raw_socket);
	test_file_details = setup_struct(reg_file);
	test_symlink_details = setup_struct(sym_link);
	test_bad_path_details = setup_struct(bad_path);
	// Create files
	if (test_pipe_details && test_pipe_details->pathname)
	{
		remove_a_file(test_pipe_details->pathname, true);  // Remove leftovers and ignore errors
		errnum = make_a_pipe(test_pipe_details->pathname);
		ck_assert_msg(0 == errnum, "make_a_pipe(%s) failed with [%d] %s",
					  test_pipe_details->pathname, errnum, strerror(errnum));
		errnum = setup_struct_mode(test_pipe_details, true);
		ck_assert_msg(0 == errnum, "setup_struct_owners(%s struct) failed with [%d] %s",
					  test_pipe_details->pathname, errnum, strerror(errnum));
	}
	else
	{
		ck_abort_msg("The setup() test fixture did not make the named pipe");
	}
	// Raw Socket
	if (test_socket_details && test_socket_details->pathname)
	{
		remove_a_file(test_socket_details->pathname, true);  // Remove leftovers and ignore errors
		errnum = make_a_socket(test_socket_details->pathname);
		ck_assert_msg(0 == errnum, "make_a_socket(%s) failed with [%d] %s",
					  test_socket_details->pathname, errnum, strerror(errnum));
		errnum = setup_struct_mode(test_socket_details, true);
		ck_assert_msg(0 == errnum, "setup_struct_owners(%s struct) failed with [%d] %s",
					  test_socket_details->pathname, errnum, strerror(errnum));
	}
	else
	{
		ck_abort_msg("The setup() test fixture did not make the socket");
	}

	// RESET OWNERSHIP
	reset_global_modes();

	// DONE
	return;
}


testPathDetails_ptr setup_struct(const char *pathname)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;                 // Errno values
	char *temp_pathname = NULL;              // Holds the resolve_test_input() return value
	testPathDetails_ptr temp_struct = NULL;  // Function return value

	// SET IT UP
	// pathname
	temp_pathname = resolve_test_input(pathname);
	// testPathDetails struct
	temp_struct = alloc_devops_mem(1, sizeof(testPathDetails), &errnum);
	ck_assert_msg(0 == errnum, "alloc_devops_mem(testPathDetails) failed with [%d] %s",
				  errnum, strerror(errnum));
	ck_assert_msg(NULL != temp_struct,
				  "alloc_devops_mem(testPathDetails) memory allocation failed");

	// STORE IT
	// pathname
	if (!errnum)
	{
		temp_struct->pathname = temp_pathname;
		errnum = setup_struct_mode(temp_struct, false);
		if (ENOENT == errnum)
		{
			errnum = 0;  // Ignore "file/dir missing" errors
		}
		ck_assert_msg(0 == errnum, "setup_struct_mode(%s struct) failed with [%d] %s",
					  temp_pathname, errnum, strerror(errnum));
	}

	// DONE
	if (errnum && temp_struct)
	{
		teardown_struct(&temp_struct);  // Something failed so undo it
	}
	return temp_struct;
}


int setup_struct_mode(testPathDetails_ptr new_struct, bool must_succeed)
{
	// LOCAL VARIABLES
	int errnum = 0;              // Errno values
	char *temp_pathname = NULL;  // Temp storage for new_struct->pathname

	// INPUT VALIDATION
	if (!new_struct)
	{
		errnum = EINVAL;  // NULL struct pointer
	}
	else
	{
		temp_pathname = new_struct->pathname;
	}

	// SETUP MODE
	// orig_mode
	if (!errnum)
	{
		new_struct->orig_mode = get_file_perms(temp_pathname, &errnum);
		if (true == must_succeed)
		{
			ck_assert_msg(0 == errnum, "get_file_perms(%s) failed with [%d] %s",
						  temp_pathname, errnum, strerror(errnum));
		}
	}

	// DONE
	return errnum;
}


void teardown(void)
{
	// RESET OWNERSHIP
	reset_global_modes();

	// CLEANUP
	// Directory
	teardown_struct(&test_dir_details);  // Ignore any errors
	// File
	teardown_struct(&test_file_details);  // Ignore any errors
	// Pipe
	remove_a_file(test_pipe_details->pathname, true);  // Best effort
	teardown_struct(&test_pipe_details);  // Ignore any errors
	// Socket
	remove_a_file(test_socket_details->pathname, true);  // Best effort
	teardown_struct(&test_socket_details);  // Ignore any errors
	// Symbolic Link
	teardown_struct(&test_symlink_details);  // Ignore any errors
	// Bad Path
	teardown_struct(&test_bad_path_details);  // Ignore any errors
}


void teardown_struct(testPathDetails_ptr *old_struct)
{
	// LOCAL VARIABLES
	int results = 0;                        // Check the results
	testPathDetails_ptr struct_ptr = NULL;  // The actual struct pointer to free

	// INPUT VALIDATION
	if (old_struct)
	{
		struct_ptr = *old_struct;
	}

	// TEAR IT DOWN
	if (struct_ptr)
	{
		// pathname
		if (struct_ptr->pathname)
		{
			results = free_devops_mem((void **)&(struct_ptr->pathname));
			ck_assert_msg(0 == results, "free_devops_mem(struct_ptr->pathname) failed with [%d] %s",
						  results, strerror(results));
		}
		// The old struct
		results = free_devops_mem((void **)old_struct);
		ck_assert_msg(0 == results, "free_devops_mem(struct_ptr) failed with [%d] %s",
					  results, strerror(results));
	}
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_directory)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_dir_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0764;  // rwxrw-r--
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_n02_named_pipe)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_pipe_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0764;  // rwxrw-r--
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_n03_regular_file)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0764;  // rwxrw-r--
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_n04_socket)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_socket_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0764;  // rwxrw-r--
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_n05_symbolic_link)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_symlink_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0764;  // rwxrw-r--
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, test_file_details->pathname, test_mode, exp_return);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_filename)
{
	// LOCAL VARIABLES
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0644;  // Test case input
	int exp_return = EINVAL;            // Expected return value for this test input
	char *input_abs_path = NULL;        // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_e02_empty_filename)
{
	// LOCAL VARIABLES
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0644;           // Test case input
	int exp_return = EINVAL;                     // Expected return value for this test input
	char input_abs_path[] = { "\0 NOT HERE!" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_e03_missing_filename)
{
	// LOCAL VARIABLES
	mode_t test_mode = TEST_MODE_0644;              // Test case input
	int exp_return = ENOENT;                        // Expected return value for this test input
	char input_abs_path[] = { "/file/not/found" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


/**************************************************************************************************/
/************************************** BOUNDARY TEST CASES ***************************************/
/**************************************************************************************************/


START_TEST(test_b01_smallest_mode)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = 0;  // ---------
	int exp_return = 0;    // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_b02_largest_mode)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_7777;  // rwsrwsrwt
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/


// Observed behavior is that, in invalid mode values, "don't care about those" bits are ignored.
START_TEST(test_s01_all_flags_enabled)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = 0xFF;  // Test case input
	int exp_return = 0;       // Expected return value for this test input

	// SETUP
	for (int i = 1; i < sizeof(mode_t); i++)
	{
		test_mode <<= 8;  // Make room for the next byte
		test_mode |= 0xFF;  // Turn on one byte's worth of bits
	}

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_s02_path_contains_non_dir)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_bad_path_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0764;  // Test case input
	int exp_return = ENOTDIR;           // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_s03_no_change)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	int errnum = CANARY_INT;                                     // Errno values
	mode_t test_mode = get_file_perms(input_abs_path, &errnum);  // Test case input
	int exp_return = 0;                                          // Expected return value

	// VALIDATE
	ck_assert_msg(0 == errnum, "get_file_perms(%s) failed with [%d] %s",
		          input_abs_path, errnum, strerror(errnum));

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_s04_special_bits)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_7764;  // rwsrwSr-T
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


START_TEST(test_s05_just_the_special_bits)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_7000;  // ---S--S--T
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


// Observed behavior is that, in invalid mode values, "don't care about those" bits are ignored.
START_TEST(test_s06_bit_field_ignores_invalid_bits)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = 077777;  // rwxrwxrwx!
	int exp_return = 0;         // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


Suite *set_mode_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SFMW_Set_Mode");   // Test suite
	TCase *tc_normal = tcase_create("Normal");      // Normal test cases
	TCase *tc_error = tcase_create("Error");        // Error test cases
	TCase *tc_boundary = tcase_create("Boundary");  // Boundary test cases
	TCase *tc_special = tcase_create("Special");    // Special test cases

	// SETUP TEST CASES
	tcase_add_checked_fixture(tc_normal, setup, teardown);
	tcase_add_checked_fixture(tc_error, setup, teardown);
	tcase_add_checked_fixture(tc_boundary, setup, teardown);
	tcase_add_checked_fixture(tc_special, setup, teardown);
	tcase_add_test(tc_normal, test_n01_directory);
	tcase_add_test(tc_normal, test_n02_named_pipe);
	tcase_add_test(tc_normal, test_n03_regular_file);
	tcase_add_test(tc_normal, test_n04_socket);
	tcase_add_test(tc_normal, test_n05_symbolic_link);
	tcase_add_test(tc_error, test_e01_null_filename);
	tcase_add_test(tc_error, test_e02_empty_filename);
	tcase_add_test(tc_error, test_e03_missing_filename);
	tcase_add_test(tc_boundary, test_b01_smallest_mode);
	tcase_add_test(tc_boundary, test_b02_largest_mode);
	tcase_add_test(tc_special, test_s01_all_flags_enabled);
	tcase_add_test(tc_special, test_s02_path_contains_non_dir);
	tcase_add_test(tc_special, test_s03_no_change);
	tcase_add_test(tc_special, test_s04_special_bits);
	tcase_add_test(tc_special, test_s05_just_the_special_bits);
	tcase_add_test(tc_special, test_s06_bit_field_ignores_invalid_bits);
	suite_add_tcase(suite, tc_normal);
	suite_add_tcase(suite, tc_error);
	suite_add_tcase(suite, tc_boundary);
	suite_add_tcase(suite, tc_special);

	return suite;
}


int main(void)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno from the function call
	// Relative path for this test case's input
	char log_rel_path[] = { "./code/test/test_output/check_sfmw_set_mode.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = set_mode_suite();
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
