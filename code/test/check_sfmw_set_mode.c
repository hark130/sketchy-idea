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
#define TEST_MODE_0444 SKID_MODE_OWNER_R | \
                       SKID_MODE_GROUP_R | \
                       SKID_MODE_OTHER_R
#define TEST_MODE_0644 SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | \
					   SKID_MODE_GROUP_R | \
                       SKID_MODE_OTHER_R
#define TEST_MODE_0754 SKID_MODE_OWNER_R | SKID_MODE_OWNER_W | SKID_MODE_OWNER_X | \
                       SKID_MODE_GROUP_R | SKID_MODE_GROUP_W | \
                       SKID_MODE_OTHER_R


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
		// Compare actual permissions to the test_mode
		ck_assert_msg(new_mode == test_mode,
			          "set_mode(%s, %o) failed to set the mode (saw %o instead)\n",
					  check_mode, test_mode, new_mode);
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

	// SET IT UP
	// Allocate structs
	test_dir_details = setup_struct(directory);
	test_pipe_details = setup_struct(named_pipe);
	test_socket_details = setup_struct(raw_socket);
	test_file_details = setup_struct(reg_file);
	test_symlink_details = setup_struct(sym_link);
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


// /**************************************************************************************************/
// /*************************************** NORMAL TEST CASES ****************************************/
// /**************************************************************************************************/


// START_TEST(test_n01_directory)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = get_new_gid();  // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_dir_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_n02_named_pipe)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = get_new_gid();  // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_pipe_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


START_TEST(test_n03_regular_file)
{
	// LOCAL VARIABLES
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;
	// New mode to use as test input
	mode_t test_mode = TEST_MODE_0754;  // rwxrw-r--
	int exp_return = 0;                 // Expected return value for this test input

	// RUN TEST
	run_test_case(input_abs_path, NULL, test_mode, exp_return);
}
END_TEST


// START_TEST(test_n04_socket)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = get_new_gid();  // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_socket_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_n05_symbolic_link_follow)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = get_new_gid();  // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_symlink_details->pathname;
// 	// Absolute path for actual_rel_path as resolved against the repo name
// 	char *actual_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, actual_abs_path, follow, new_uid, new_gid);
// }
// END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


// START_TEST(test_e01_null_filename)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = get_new_gid();  // Test case input
// 	char *input_abs_path = NULL;    // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_e02_empty_filename)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                          // Test case input
// 	uid_t new_uid = get_new_uid();               // Test case input
// 	gid_t new_gid = get_new_gid();               // Test case input
// 	char input_abs_path[] = { "\0 NOT HERE!" };  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_e03_missing_filename)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                             // Test case input
// 	uid_t new_uid = get_new_uid();                  // Test case input
// 	gid_t new_gid = get_new_gid();                  // Test case input
// 	char input_abs_path[] = { "/file/not/found" };  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


/**************************************************************************************************/
/************************************** BOUNDARY TEST CASES ***************************************/
/**************************************************************************************************/


// // Minimum GID for a system user (e.g., root, bin, sys)
// START_TEST(test_b01_min_good_system_user_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 0;              // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Maximum GID for a system user (e.g., root, bin, sys)
// START_TEST(test_b02_max_good_system_user_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 99;             // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Minimum GID for an application user (e.g., crontab, syslog, pulse)
// START_TEST(test_b03_min_good_application_user_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 100;            // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Maximum GID for an application user (e.g., crontab, syslog, pulse)
// START_TEST(test_b04_max_good_system_user_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 999;            // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Minimum GID for a regular user (see: useradd mumu)
// START_TEST(test_b05_min_good_regular_user_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 1000;           // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Maximum GID for Debian: https://www.baeldung.com/linux/user-ids-reserved-values
// START_TEST(test_b06_max_good_debian_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 59999;          // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Maximum GID for Solaris: https://docs.oracle.com/cd/E19455-01/805-7228/userconcept-35/index.html
// START_TEST(test_b07_max_good_solaris_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 60000;          // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// // Maximum GID for Linux (see: grep ^GID_MAX /etc/login.defs)
// START_TEST(test_b08_max_good_linux_gid)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;             // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = 60000;          // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// /**************************************************************************************************/
// /*************************************** SPECIAL TEST CASES ***************************************/
// /**************************************************************************************************/


// START_TEST(test_s01_symbolic_link_no_follow)
// {
// 	// LOCAL VARIABLES
// 	bool follow = false;            // Test case input
// 	uid_t new_uid = get_new_uid();  // Test case input
// 	gid_t new_gid = get_new_gid();  // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_symlink_details->pathname;

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_s02_nobody_uid)
// {
// 	// LOCAL VARIABLES
// 	int errnum = 0;                                         // Errno value
// 	bool follow = true;                                     // Test case input
// 	uid_t new_uid = get_shell_user_uid("nobody", &errnum);  // Test case input
// 	gid_t new_gid = CSSO_SKIP_GID;                          // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// VALIDATE
// 	ck_assert_msg(0 == errnum, "get_shell_user_uid() failed with [%d] %s",
// 		          errnum, strerror(errnum));

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_s03_nogroup_gid)
// {
// 	// LOCAL VARIABLES
// 	int errnum = 0;                                          // Errno value
// 	bool follow = true;                                      // Test case input
// 	uid_t new_uid = get_new_uid();                           // Test case input
// 	gid_t new_gid = get_shell_user_gid("nobody", &errnum);  // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// VALIDATE
// 	ck_assert_msg(0 == errnum, "get_shell_user_gid() failed with [%d] %s",
// 		          errnum, strerror(errnum));

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


// START_TEST(test_s04_nobody_nogroup)
// {
// 	// LOCAL VARIABLES
// 	int errnum = 0;      // Errno value
// 	bool follow = true;  // Test case input
// 	uid_t new_uid = 0;   // Test case input
// 	gid_t new_gid = 0;   // Test case input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_file_details->pathname;

// 	// SETUP
// 	// UID
// 	new_uid = get_shell_user_uid("nobody", &errnum);
// 	ck_assert_msg(0 == errnum, "get_shell_user_uid() failed with [%d] %s",
// 		          errnum, strerror(errnum));
// 	// GID
// 	new_gid = get_shell_user_gid("nobody", &errnum);
// 	ck_assert_msg(0 == errnum, "get_shell_user_gid() failed with [%d] %s",
// 		          errnum, strerror(errnum));

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
// }
// END_TEST


Suite *set_mode_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SFMW_Set_Mode");  // Test suite
	TCase *tc_normal = tcase_create("Normal");       // Normal test cases
	// TCase *tc_error = tcase_create("Error");         // Error test cases
	// TCase *tc_boundary = tcase_create("Boundary");   // Boundary test cases
	// TCase *tc_special = tcase_create("Special");     // Special test cases

	// SETUP TEST CASES
	tcase_add_checked_fixture(tc_normal, setup, teardown);
	// tcase_add_checked_fixture(tc_error, setup, teardown);
	// tcase_add_checked_fixture(tc_boundary, setup, teardown);
	// tcase_add_checked_fixture(tc_special, setup, teardown);
	// tcase_add_test(tc_normal, test_n01_directory);
	// tcase_add_test(tc_normal, test_n02_named_pipe);
	tcase_add_test(tc_normal, test_n03_regular_file);
	// tcase_add_test(tc_normal, test_n04_socket);
	// tcase_add_test(tc_normal, test_n05_symbolic_link_follow);
	// tcase_add_test(tc_error, test_e01_null_filename);
	// tcase_add_test(tc_error, test_e02_empty_filename);
	// tcase_add_test(tc_error, test_e03_missing_filename);
	// tcase_add_test(tc_boundary, test_b01_min_good_system_user_gid);
	// tcase_add_test(tc_boundary, test_b02_max_good_system_user_gid);
	// tcase_add_test(tc_boundary, test_b03_min_good_application_user_gid);
	// tcase_add_test(tc_boundary, test_b04_max_good_system_user_gid);
	// tcase_add_test(tc_boundary, test_b05_min_good_regular_user_gid);
	// tcase_add_test(tc_boundary, test_b06_max_good_debian_gid);
	// tcase_add_test(tc_boundary, test_b07_max_good_solaris_gid);
	// tcase_add_test(tc_boundary, test_b08_max_good_linux_gid);
	// tcase_add_test(tc_special, test_s01_symbolic_link_no_follow);
	// tcase_add_test(tc_special, test_s02_nobody_uid);
	// tcase_add_test(tc_special, test_s03_nogroup_gid);
	// tcase_add_test(tc_special, test_s04_nobody_nogroup);
	suite_add_tcase(suite, tc_normal);
	// suite_add_tcase(suite, tc_error);
	// suite_add_tcase(suite, tc_boundary);
	// suite_add_tcase(suite, tc_special);

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
