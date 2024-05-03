/*
 *  Check unit test suit for skid_file_metadata_write.h's set_mtime_now() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmw_set_mtime_now.bin
code/dist/check_sfmw_set_mtime_now.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmw_set_mtime_now.bin

 *
 */

#include <check.h>                     // START_TEST(), END_TEST
#include <limits.h>                    // PATH_MAX
#include <stdio.h>                     // printf()
#include <stdlib.h>
#include <unistd.h>                    // get_current_dir_name(), usleep()
// Local includes
#include "devops_code.h"               // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"   // get_access_time()
#include "skid_file_metadata_write.h"  // set_mtime_now()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
// Use this with usleep(MICRO_SEC_SLEEP) to let the file timestamp update
#define MICRO_SEC_SLEEP (useconds_t)10000  // Give the file timestamp 0.01 second to update


/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

char *test_pipe_path;  // Heap array containing the absolute pipe filename resolved to the repo
char *test_socket_path;  // Heap array containing the absolute socket filename resolved to the repo

/*
 *	Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *	the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *	1. Gets the original time, 2. Calls the tested function, 3. Gets the new time.
 *	Verifies: actual return == exp_ret, original time < new time (special cases
 *	original time <= new time for directories because of "filesystem relatime flag").
 *	If follow_sym is true and pathname is a symbolic link, checks the before and after timestamps
 *	of target_name instead.  Otherwise, target_name is ignored.
 */
void run_test_case(const char *pathname, const char *target_name, bool follow_sym, int exp_ret);

/*
 *	Resolve the named pipe and raw socket default filenames to the repo and store the heap
 *	memory pointer in the globals.
 */
void setup(void);

/*
 *	Delete the named pipe and raw socket files.  Then, free the heap memory arrays.
 */
void teardown(void);

/*
 *	Compare the old vs new times to verify the timestamp was updated.
 */
void verify_time_update(const char *pathname, const char *time_adj,
	                    time_t old_time, long old_time_nsec, time_t new_time, long new_time_nsec);


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


void run_test_case(const char *pathname, const char *target_name, bool follow_sym, int exp_ret)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;            // Errno values
	time_t old_time = 0;                // Original time
	long old_time_nsec = 0;             // Original time nanoseconds
	time_t new_time = 0;                // New time
	long new_time_nsec = 0;             // New time nanoseconds
	const char *time_check = pathname;  // Filename to gather timestamps for

	// CHECK IT
	// Should we use target_name instead?
	if (pathname && *pathname && true == follow_sym && true == is_sym_link(pathname, &errnum))
	{
		ck_assert_msg(0 == errnum, "is_sym_link(%s) failed with [%d] %s",
					  pathname, errnum, strerror(errnum));
		time_check = target_name;
	}
	errnum = CANARY_INT;  // Reset temp variable

	// RUN IT
	// No need to compare times if the expected result is "error"
	if (0 == exp_ret)
	{
		// 1. Get original time
		errnum = get_mod_timestamp(time_check, &old_time, &old_time_nsec, follow_sym);
		ck_assert_msg(0 == errnum, "old get_mod_timestamp(%s) failed with [%d] %s",
					  time_check, errnum, strerror(errnum));
		errnum = CANARY_INT;  // Reset temp variable
	}
	// 2. Call function
	micro_sleep(MICRO_SEC_SLEEP);  // Wait before changing the timestamp
	errnum = set_mtime_now(pathname, follow_sym);
	ck_assert_msg(exp_ret == errnum, "set_mtime_now(%s) returned [%d] '%s' instead of [%d] '%s",
		          pathname, errnum, strerror(errnum), exp_ret, strerror(exp_ret));
	errnum = CANARY_INT;  // Reset temp variable
	// No need to compare times if the expected result is "error"
	if (0 == exp_ret)
	{
		// 3. Get current time
		errnum = get_mod_timestamp(time_check, &new_time, &new_time_nsec, follow_sym);
		ck_assert_msg(0 == errnum, "new get_mod_timestamp(%s) failed with [%d] %s",
					  time_check, errnum, strerror(errnum));

		// VALIDATE RESULTS
		verify_time_update(time_check, "modification", old_time, old_time_nsec,
			               new_time, new_time_nsec);
	}
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


void verify_time_update(const char *pathname, const char *time_adj,
	                    time_t old_time, long old_time_nsec, time_t new_time, long new_time_nsec)
{
	// Compare times
	if (old_time > new_time)
	{
		ck_abort_msg("The %s time for %s was not updated: old sec %ld > new sec %ld",
		             time_adj, pathname, old_time, new_time);
	}
	else if (old_time == new_time && old_time_nsec >= new_time_nsec)
	{
		ck_abort_msg("The %s nanoseconds for %s weren't updated: old nsec %ld >= new nsec %ld",
		             time_adj, pathname, old_time_nsec, new_time_nsec);
	}
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_directory)
{
	// LOCAL VARIABLES
	bool follow = true;  // Follow symlinks
	int exp_result = 0;  // Expected results
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/");

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_n02_named_pipe)
{
	// LOCAL VARIABLES
	bool follow = true;  // Follow symlinks
	int exp_result = 0;  // Expected results
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_pipe_path;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


START_TEST(test_n03_regular_file)
{
	// LOCAL VARIABLES
	bool follow = true;  // Follow symlinks
	int exp_result = 0;  // Expected results
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/regular_file.txt");

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
}
END_TEST


START_TEST(test_n04_socket)
{
	// LOCAL VARIABLES
	bool follow = true;  // Follow symlinks
	int exp_result = 0;  // Expected results
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_socket_path;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


START_TEST(test_n05_symbolic_link_follow)
{
	// LOCAL VARIABLES
	bool follow = true;  // Follow symlinks
	int exp_result = 0;  // Expected results
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/sym_link.txt");
	// Absolute path for actual_rel_path as resolved against the repo name
	char *actual_abs_path = resolve_test_input("./code/test/test_input/regular_file.txt");

	// RUN TEST
	run_test_case(input_abs_path, actual_abs_path, follow, exp_result);

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
	// LOCAL VARIABLES
	bool follow = true;           // Follow symlinks
	int exp_result = EINVAL;      // Expected results
	char *input_abs_path = NULL;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


START_TEST(test_e02_empty_filename)
{
	// LOCAL VARIABLES
	bool follow = true;                          // Follow symlinks
	int exp_result = EINVAL;                     // Expected results
	char input_abs_path[] = { "\0 NOT HERE!" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/
START_TEST(test_s01_missing_filename)
{
	// LOCAL VARIABLES
	bool follow = true;                                 // Follow symlinks
	int exp_result = ENOENT;                            // Expected results
	char input_abs_path[] = { "/does/not/exist.txt" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


START_TEST(test_s02_symbolic_link_nofollow)
{
	// LOCAL VARIABLES
	bool follow = false;  // Follow symlinks
	int exp_result = 0;   // Expected results
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = resolve_test_input("./code/test/test_input/sym_link.txt");
	// Absolute path for actual_rel_path as resolved against the repo name
	char *actual_abs_path = resolve_test_input("./code/test/test_input/regular_file.txt");

	// RUN TEST
	run_test_case(input_abs_path, actual_abs_path, follow, exp_result);

	// CLEANUP
	free_devops_mem((void **)&input_abs_path);
	free_devops_mem((void **)&actual_abs_path);
}
END_TEST
 

START_TEST(test_s03_block_device)
{
	// LOCAL VARIABLES
	bool follow = true;                        // Follow symlinks
	int exp_result = EPERM;                    // Expected results
	char input_abs_path[] = { "/dev/loop0" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


START_TEST(test_s04_character_device)
{
	// LOCAL VARIABLES
	bool follow = true;                       // Follow symlinks
	int exp_result = EPERM;                   // Expected results
	char input_abs_path[] = { "/dev/null" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result);
}
END_TEST


Suite *set_mtime_now_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_core = NULL;

	suite = suite_create("SFMW_Set_Mtime_Now");

	/* Core test case */
	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_n01_directory);
	tcase_add_test(tc_core, test_n02_named_pipe);
	tcase_add_test(tc_core, test_n03_regular_file);
	tcase_add_test(tc_core, test_n04_socket);
	tcase_add_test(tc_core, test_n05_symbolic_link_follow);
	tcase_add_test(tc_core, test_e01_null_filename);
	tcase_add_test(tc_core, test_e02_empty_filename);
	tcase_add_test(tc_core, test_s01_missing_filename);
	tcase_add_test(tc_core, test_s02_symbolic_link_nofollow);
	tcase_add_test(tc_core, test_s03_block_device);
	tcase_add_test(tc_core, test_s04_character_device);
	suite_add_tcase(suite, tc_core);

	return suite;
}


int main(void)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno from the function call
	// Relative path for this test case's input
	char log_rel_path[] = { "./code/test/test_output/check_sfmw_set_mtime_now.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = set_mtime_now_suite();
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
