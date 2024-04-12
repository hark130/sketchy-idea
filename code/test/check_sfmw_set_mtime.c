/*
 *  Check unit test suit for skid_file_metadata_write.h's set_mtime() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmw_set_mtime.bin
code/dist/check_sfmw_set_mtime.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmw_set_mtime.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:

export CK_RUN_CASE="Normal" && ./code/dist/check_sfmw_set_mtime.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sfmw_set_mtime.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sfmw_set_mtime.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sfmw_set_mtime.bin; unset CK_RUN_CASE  # Just run the Special test cases

 */

#include <check.h>                     // START_TEST(), END_TEST
#include <limits.h>                    // PATH_MAX
#include <stdio.h>                     // printf()
#include <stdlib.h>
#include <unistd.h>                    // get_current_dir_name(), usleep()
// Local includes
#include "devops_code.h"               // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"   // get_mod_time()
#include "skid_file_metadata_write.h"  // set_mtime()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
// Use this with usleep(MICRO_SEC_SLEEP) to let the file timestamp update
#define MICRO_SEC_SLEEP (useconds_t)10000  // Give the file timestamp 0.01 second to update
#define UTIME_NSEC_MIN 0l          // Minimum allowed value for tv_nsec (see: utimensat(2))
#define UTIME_NSEC_MAX 999999999l  // Maximum allowed value for tv_nsec (see: utimensat(2))


/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/


char *test_dir_path;  // Heap array containing the absolute directory name resolved to the repo
char *test_file_path;  // Heap array containing the absolute filename resolved to the repo
char *test_pipe_path;  // Heap array containing the absolute pipe filename resolved to the repo
char *test_socket_path;  // Heap array containing the absolute socket filename resolved to the repo
char *test_sym_link;  // Heap array with the absolute symbolic link filename resolved to the repo

/*
 *	Set the atime and mtime timestamps of the test_*_path globals to "now".
 */
void reset_global_timestamps(void);

/*
 *	Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *	the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *	1. Gets the original time, 2. Calls the tested function, 3. Gets the new time.
 *	Verifies: actual return == exp_ret, desired time == current time
 *	If follow_sym is true and pathname is a symbolic link, checks the before and after timestamps
 *	of target_name instead.  Otherwise, target_name is ignored.
 */
void run_test_case(const char *pathname, const char *target_name, bool follow_sym, int exp_ret,
	               time_t new_sec, long new_nsec);

/*
 *	Resolve all the default test filenames to the repo and store the heap memory pointer in
 *	the globals.  Also, set the timestamps to now.
 */
void setup(void);

/*
 *	Delete the named pipe and raw socket files.  Reset the timestamps for the remaining globals.
 *	Then, free the heap memory arrays.
 */
void teardown(void);

/*
 *	Compare the desired time vs current times to verify the timestamp was updated.
 */
void verify_time_update(const char *pathname, const char *time_adj,
						time_t new_time, long new_time_nsec, time_t curr_time, long curr_time_nsec);


void reset_global_timestamps(void)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;  // Errno values
	// Array of the global pointers to iterate over
	char *test_paths[] = {test_dir_path, test_file_path, test_pipe_path,
	                      test_socket_path, test_sym_link};

	// RESET TIMES
	for (int i = 0; i < (sizeof(test_paths) / sizeof(test_paths[0])); i++)
	{
		errnum = set_times_now(test_paths[i], false);
		ck_assert_msg(0 == errnum, "set_times_now(%s) failed with [%d] %s", test_paths[i],
			          errnum, strerror(errnum));
	}

	// DONE
	return;
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
		free_devops_mem(&resolved_name);  // Best effort
	}
	return resolved_name;
}


void run_test_case(const char *pathname, const char *target_name, bool follow_sym, int exp_ret,
	               time_t new_sec, long new_nsec)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;            // Errno values
	time_t old_time = 0;                // Original time
	long old_time_nsec = 0;             // Original time nanoseconds
	time_t curr_time = 0;               // Current time
	long curr_time_nsec = 0;            // Current time nanoseconds
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
	errnum = set_mtime(pathname, follow_sym, new_sec, new_nsec);
	ck_assert_msg(exp_ret == errnum, "set_mtime(%s) returned [%d] '%s' instead of [%d] '%s'",
				  pathname, errnum, strerror(errnum), exp_ret, strerror(exp_ret));
	errnum = CANARY_INT;  // Reset temp variable
	// No need to compare times if the expected result is "error"
	if (0 == exp_ret)
	{
		// 3. Get current time
		errnum = get_mod_timestamp(time_check, &curr_time, &curr_time_nsec, follow_sym);
		ck_assert_msg(0 == errnum, "new get_mod_timestamp(%s) failed with [%d] %s",
					  time_check, errnum, strerror(errnum));

		// VALIDATE RESULTS
		verify_time_update(time_check, "modification", new_sec, new_nsec, curr_time, curr_time_nsec);
	}
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
	// Directory
	test_dir_path = resolve_test_input(directory);
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
	// Regular File
	test_file_path = resolve_test_input(reg_file);
	// Symbolic Link
	test_sym_link = resolve_test_input(sym_link);

	// RESET TIMESTAMPS
	reset_global_timestamps();

	// DONE
	return;
}


void teardown(void)
{
	// Directory
	set_times_now(test_dir_path, false);  // Ignore any errors
	free_devops_mem(&test_dir_path);  // Ignore any errors
	// File
	set_times_now(test_file_path, false);  // Ignore any errors
	free_devops_mem(&test_file_path);  // Ignore any errors
	// Pipe
	remove_a_file(test_pipe_path, true);  // Best effort
	free_devops_mem(&test_pipe_path);  // Ignore any errors
	// Socket
	remove_a_file(test_socket_path, true);  // Best effort
	free_devops_mem(&test_socket_path);  // Ignore any errors
	// Symbolic Link
	set_times_now(test_sym_link, false);  // Ignore any errors
	free_devops_mem(&test_sym_link);  // Ignore any errors
}


void verify_time_update(const char *pathname, const char *time_adj,
						time_t new_time, long new_time_nsec, time_t curr_time, long curr_time_nsec)
{
	// Compare times
	if (new_time != curr_time)
	{
		ck_abort_msg("The %s time for %s was not updated: new sec %ld != curr sec %ld",
					 time_adj, pathname, new_time, curr_time);
	}
	else if (new_time_nsec != curr_time_nsec)
	{
		ck_abort_msg("The %s nanoseconds for %s weren't updated: new nsec %ld != curr nsec %ld",
					 time_adj, pathname, new_time_nsec, curr_time_nsec);
	}
}


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_n01_directory)
{
	// LOCAL VARIABLES
	bool follow = true;           // Follow symlinks
	int exp_result = 0;           // Expected results
	time_t new_sec = 0x600DC0DE;  // Seconds test input
	long new_nsec = 0xD06F00D;    // Nanoseconds test input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_dir_path;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_n02_named_pipe)
{
	// LOCAL VARIABLES
	bool follow = true;         // Follow symlinks
	int exp_result = 0;         // Expected results
	time_t new_sec = 0xBAD71E;  // Seconds test input
	long new_nsec = 0x347F00D;  // Nanoseconds test input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_pipe_path;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_n03_regular_file)
{
	// LOCAL VARIABLES
	bool follow = true;         // Follow symlinks
	int exp_result = 0;         // Expected results
	time_t new_sec = 0xB00000;  // Seconds test input
	long new_nsec = 0xC0010FF;  // Nanoseconds test input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_path;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_n04_socket)
{
	// LOCAL VARIABLES
	bool follow = true;       // Follow symlinks
	int exp_result = 0;       // Expected results
	time_t new_sec = 0xD34D;  // Seconds test input
	long new_nsec = 0xBEEF;   // Nanoseconds test input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_socket_path;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_n05_symbolic_link_follow)
{
	// LOCAL VARIABLES
	bool follow = true;       // Follow symlinks
	int exp_result = 0;       // Expected results
	time_t new_sec = 0xFEED;  // Seconds test input
	long new_nsec = 0xFACE;   // Nanoseconds test input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_sym_link;
	// Absolute path for actual_rel_path as resolved against the repo name
	char *actual_abs_path = test_file_path;

	// RUN TEST
	run_test_case(input_abs_path, actual_abs_path, follow, exp_result, new_sec, new_nsec);
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
	time_t new_sec = 0x7E57;      // Seconds test input
	long new_nsec = 0xC0DE;       // Nanoseconds test input
	char *input_abs_path = NULL;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_e02_empty_filename)
{
	// LOCAL VARIABLES
	bool follow = true;                          // Follow symlinks
	int exp_result = EINVAL;                     // Expected results
	time_t new_sec = 0x7E57;                     // Seconds test input
	long new_nsec = 0xC0DE;                      // Nanoseconds test input
	char input_abs_path[] = { "\0 NOT HERE!" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


/**************************************************************************************************/
/************************************** BOUNDARY TEST CASES ***************************************/
/**************************************************************************************************/
START_TEST(test_b01_sec_min_value)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = INT_MIN + 1;           // Seconds test input
	long new_nsec = 0x7E57;                 // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b02_sec_almost_epoch)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = -1;                    // Seconds test input
	long new_nsec = 0x7E57;                 // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b03_sec_epoch_time)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = 0;                     // Seconds test input
	long new_nsec = 0x7E57;                 // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b04_sec_barely_past_epoch)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = 1;                     // Seconds test input
	long new_nsec = 0x7E57;                 // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b05_sec_static_now)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = 0x66197EA4;            // Seconds test input: 4/12/2024, 1:34:12 PM
	long new_nsec = 0x7E57;                 // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b06_sec_max_value)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = INT_MAX;               // Seconds test input
	long new_nsec = 0x7E57;                 // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b07_nsec_low_very_bad)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = EINVAL;                // Expected results
	time_t new_sec = 0x7E57;                // Seconds test input
	long new_nsec = LONG_MIN;               // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b08_nsec_low_barely_bad)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = EINVAL;                // Expected results
	time_t new_sec = 0x7E57;                // Seconds test input
	long new_nsec = UTIME_NSEC_MIN - 1l;    // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b09_nsec_low_barely_good)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = 0x7E57;                // Seconds test input
	long new_nsec = UTIME_NSEC_MIN;         // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b10_nsec_high_barely_good)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = 0x7E57;                // Seconds test input
	long new_nsec = UTIME_NSEC_MAX;         // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b11_nsec_high_barely_bad)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = EINVAL;                // Expected results
	time_t new_sec = 0x7E57;                // Seconds test input
	long new_nsec = UTIME_NSEC_MAX + 1l;    // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_b12_nsec_high_very_bad)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = EINVAL;                // Expected results
	time_t new_sec = 0x7E57;                // Seconds test input
	long new_nsec = LONG_MAX;               // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
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
	time_t new_sec = 0x7E57;                            // Seconds test input
	long new_nsec = 0xC0DE;                             // Nanoseconds test input
	char input_abs_path[] = { "/does/not/exist.txt" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_s02_symbolic_link_nofollow)
{
	// LOCAL VARIABLES
	bool follow = false;        // Follow symlinks
	int exp_result = 0;         // Expected results
	time_t new_sec = 0x4B1D;    // Seconds test input
	long new_nsec = 0xB16C0DE;  // Nanoseconds test input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_sym_link;
	// Absolute path for actual_rel_path as resolved against the repo name
	char *actual_abs_path = test_file_path;

	// RUN TEST
	run_test_case(input_abs_path, actual_abs_path, follow, exp_result, new_sec, new_nsec);
}
END_TEST
 

START_TEST(test_s03_block_device)
{
	// LOCAL VARIABLES
	bool follow = true;                        // Follow symlinks
	int exp_result = EPERM;                    // Expected results
	time_t new_sec = 0x7E57;                   // Seconds test input
	long new_nsec = 0xC0DE;                    // Nanoseconds test input
	char input_abs_path[] = { "/dev/loop0" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


START_TEST(test_s04_character_device)
{
	// LOCAL VARIABLES
	bool follow = true;                       // Follow symlinks
	int exp_result = EPERM;                   // Expected results
	time_t new_sec = 0x7E57;                  // Seconds test input
	long new_nsec = 0xC0DE;                   // Nanoseconds test input
	char input_abs_path[] = { "/dev/null" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


/*
 *	NOTE: If the seconds are set to INT_MIN, the nanoseconds appear to always be set to 0.
 */
START_TEST(test_s05_sec_min_value_edge_case)
{
	// LOCAL VARIABLES
	bool follow = true;                     // Follow symlinks
	int exp_result = 0;                     // Expected results
	time_t new_sec = INT_MIN;               // Seconds test input
	long new_nsec = 0;                      // Nanoseconds test input
	char *input_abs_path = test_file_path;  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
}
END_TEST


Suite *set_mtime_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SFMW_Set_Mtime");  // Test suite
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
	tcase_add_test(tc_normal, test_n05_symbolic_link_follow);
	tcase_add_test(tc_error, test_e01_null_filename);
	tcase_add_test(tc_error, test_e02_empty_filename);
	tcase_add_test(tc_boundary, test_b01_sec_min_value);
	tcase_add_test(tc_boundary, test_b02_sec_almost_epoch);
	tcase_add_test(tc_boundary, test_b03_sec_epoch_time);
	tcase_add_test(tc_boundary, test_b04_sec_barely_past_epoch);
	tcase_add_test(tc_boundary, test_b05_sec_static_now);
	tcase_add_test(tc_boundary, test_b06_sec_max_value);
	tcase_add_test(tc_boundary, test_b07_nsec_low_very_bad);
	tcase_add_test(tc_boundary, test_b08_nsec_low_barely_bad);
	tcase_add_test(tc_boundary, test_b09_nsec_low_barely_good);
	tcase_add_test(tc_boundary, test_b10_nsec_high_barely_good);
	tcase_add_test(tc_boundary, test_b11_nsec_high_barely_bad);
	tcase_add_test(tc_boundary, test_b12_nsec_high_very_bad);
	tcase_add_test(tc_special, test_s01_missing_filename);
	tcase_add_test(tc_special, test_s02_symbolic_link_nofollow);
	tcase_add_test(tc_special, test_s03_block_device);
	tcase_add_test(tc_special, test_s04_character_device);
	tcase_add_test(tc_special, test_s05_sec_min_value_edge_case);
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
	char log_rel_path[] = { "./code/test/test_output/check_sfmw_set_mtime.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;         // Number of test failures
	Suite *suite = NULL;           // Test suite
	SRunner *suite_runner = NULL;  // Test suite runner

	// SETUP
	suite = set_mtime_suite();
	suite_runner = srunner_create(suite);
	srunner_set_log(suite_runner, log_abs_path);

	// RUN IT
	srunner_run_all(suite_runner, CK_NORMAL);
	number_failed = srunner_ntests_failed(suite_runner);

	// CLEANUP
	srunner_free(suite_runner);
	free_devops_mem(&log_abs_path);

	// DONE
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
