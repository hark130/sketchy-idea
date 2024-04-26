/*
 *  Check unit test suit for skid_file_metadata_write.h's set_ownership() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmw_set_ownership.bin && \
code/dist/check_sfmw_set_ownership.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmw_set_ownership.bin

 *
 *	The test cases have been split up by normal, error, boundary, and special (NEBS).
 *	Execute this command to run just one NEBS category:
 *

export CK_RUN_CASE="Normal" && ./code/dist/check_sfmw_set_ownership.bin; unset CK_RUN_CASE  # Just run the Normal test cases
export CK_RUN_CASE="Error" && ./code/dist/check_sfmw_set_ownership.bin; unset CK_RUN_CASE  # Just run the Error test cases
export CK_RUN_CASE="Boundary" && ./code/dist/check_sfmw_set_ownership.bin; unset CK_RUN_CASE  # Just run the Boundary test cases
export CK_RUN_CASE="Special" && ./code/dist/check_sfmw_set_ownership.bin; unset CK_RUN_CASE  # Just run the Special test cases

 *
 */

#include <check.h>						// START_TEST(), END_TEST
#include <limits.h>						// PATH_MAX
#include <stdio.h>						// printf()
#include <stdlib.h>
#include <unistd.h>						// get_current_dir_name(), usleep()
// Local includes
#include "devops_code.h"				// resolve_to_repo(), SKID_REPO_NAME
#ifndef SKID_DEBUG
#define SKID_DEBUG
#endif  /* SKID_DEBUG */
#include "skid_debug.h"     			// FPRINTF_ERR()
#include "skid_file_metadata_read.h"	// get_access_time()
#include "skid_file_metadata_write.h"	// set_ownership()


/**************************************************************************************************/
/******* DEFINE THESE CSSO_DEF_?ID MACROS TO OVERRIDE DEFAULT IDS USED IN THESE TEST CASES ********/
/**************************************************************************************************/
// GENERAL UNIT TEST NOTE: These unit tests will default to "id -u" for the UID and attempt to
//	programmatically determine a compatible GID (see: "grep `whoami` /etc/group")
// UID PRO TIP: Take care if you choose a UID other than "id -u" because you'll need to enable
//	the CAP_CHOWN capability (see: capabilities(7)) or elevate privileges.
// #define CSSO_DEF_UID 1234          // Check skid_file_metadata_write default owner ID
// GID PRO TIP: If you choose a GID, choose an ID for a group in grep `whoami` /etc/group
// #define CSSO_DEF_GID CSSO_DEF_UID  // Check skid_file_metadata_write default group ID


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value
#define CSSO_SKIP_UID (uid_t)-1  // Do not update the UID
#define CSSO_SKIP_GID (gid_t)-1  // Do not update the GID


/**************************************************************************************************/
/***************************************** TEST FIXTURES ******************************************/
/**************************************************************************************************/

// This struct defines details about test files to aid in test execution and teardown() recovery.
typedef struct _testPathDetails
{
	char *pathname;    // Heap-allocated buffer containing the pathname resolved to the repo dir
	uid_t orig_owner;  // The original owner's UID
	gid_t orig_group;  // The original group's GID
} testPathDetails, *testPathDetails_ptr;

testPathDetails_ptr test_dir_details;  // Heap-allocated struct containing test directory details
testPathDetails_ptr test_file_details;  // Heap-allocated struct containing test filename details
testPathDetails_ptr test_pipe_details;  // Heap-allocated struct containing test pipe details
testPathDetails_ptr test_socket_details;  // Heap-allocated struct containing test socket details
testPathDetails_ptr test_symlink_details;  // Heap-allocated struct with test symbolic link details

/*
 *	Answers the question, "Is my test_uid a member of the gid_club GID?"
 */
bool compare_compatibility(uid_t test_uid, gid_t gid_club);

/*
 *	Compare pathname's actual UID and GID to the expected values.
 */
void compare_ownership(const char *pathname, uid_t exp_uid, gid_t exp_gid);

/*
 *	Programatically determine what the expected return value will be.
 */
int determine_exp_return(const char *pathname, uid_t exp_uid, gid_t exp_gid);

/*
 *	Programmatically determine a new GID for pathname.  Priority order:
 *		CSSO_DEF_GID, get_shell_compatible_gid(), the GID of the owning process.
 */
gid_t get_new_gid(void);

/*
 *	Set the ownership, UID and GID, of the test_*_details globals as specified.  If take_over is
 *	true, this function will ignore the struct's UIDs/GIDs and use the UID/GID of the owning
 *	process, by using get_shell_my_uid() and get_shell_my_gid() respectively, instead.
 */
void reset_global_ownership(bool take_over);

/*
 *	Resolve paththame to SKID_REPO_NAME in a standardized way.  Use free_devops_mem() to free
 *	the return value.
 */
char *resolve_test_input(const char *pathname);

/*
 *	
 */
void run_test_case(const char *pathname, const char *target_name, bool follow_sym,
				   uid_t new_uid, gid_t new_gid);

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
 *	Update new_struct with the owner and group for new_struct->pathname.  Does not ignore errors
 *	if must_succeed is true.  Always returns the error received.
 */
int setup_struct_owners(testPathDetails_ptr new_struct, bool must_succeed);

/*
 *	Delete the named pipe and raw socket files.  Then, free the heap memory arrays.
 */
void teardown(void);

/*
 *	Free all allocate memory in the testPathDetails struct in a gently tolerant way.
 */
void teardown_struct(testPathDetails_ptr* old_struct);


bool compare_compatibility(uid_t test_uid, gid_t gid_club)
{
	// LOCAL VARIABLES
	int errnum = 0;             // Errno values
	bool is_member = false;     // True if username is a member of gid_club
	gid_t *compat_gids = NULL;  // Compatible GIDs
	gid_t *tmp_gid = NULL;      // Iterating pointer
	gid_t last_gid = 0;         // Last GID we saw in the array

	// COMPARE IT
	compat_gids = get_shell_compatible_gid(&errnum);
	ck_assert_msg(0 == errnum, "get_shell_compatible_gid() failed with [%d] %s",
		          errnum, strerror(errnum));
	ck_assert_msg(NULL != compat_gids, "get_shell_compatible_gid() silently failed with NULL");
	if (compat_gids)
	{
		tmp_gid = compat_gids;
		while(1)
		{
			if (gid_club == *tmp_gid)
			{
				is_member = true;  // We're in!
				break;  // We got our answer
			}
			else if (test_uid == *tmp_gid)
			{
				break;  // End of the array?  Bail to be safe.
			}
			else
			{
				last_gid = *tmp_gid;  // Store the current, soon to be last, GID
				tmp_gid++;  // Advance to next GID in the array
				if (last_gid == *tmp_gid)
				{
					break;  // Edge of the cliff... BAIL!
				}
			}
		}
	}

	// CLEANUP
	if (compat_gids)
	{
		free_devops_mem((void **)&compat_gids);
	}

	// DONE
	return is_member;
}


void compare_ownership(const char *pathname, uid_t exp_uid, gid_t exp_gid)
{
	// LOCAL VARIABLES
	int errnum = 0;      // Errno values
	uid_t curr_uid = 0;  // Current UID
	gid_t curr_gid = 0;  // Current GID

	// COMPARE IT
	// 1. Get current ownership
	// UID
	curr_uid = get_owner(pathname, &errnum);
	ck_assert_msg(0 == errnum, "get_owner(%s) failed with [%d] %s", pathname,
		          errnum, strerror(errnum));
	// GID
	curr_gid = get_group(pathname, &errnum);
	ck_assert_msg(0 == errnum, "get_group(%s) failed with [%d] %s", pathname,
		          errnum, strerror(errnum));
	// 2. Compare ownership
	ck_assert_msg(curr_uid == exp_uid, "%s has a UID of %u but was expected to have %u",
				  pathname, curr_uid, exp_uid);
	ck_assert_msg(curr_gid == exp_gid, "%s has a GID of %u but was expected to have %u",
				  pathname, curr_gid, exp_gid);

	// DONE
	return;
}


int determine_exp_return(const char *pathname, uid_t exp_uid, gid_t exp_gid)
{
	// LOCAL VARIABLES
	int errnum = 0;                            // Errno values
	int exp_return = 0;                        // Expected return value
	uid_t my_uid = get_shell_my_uid(&errnum);  // My UID
	uid_t path_owner = 0;                      // UID of pathname

	// INPUT VALIDATION
	ck_assert_msg(0 == errnum, "get_shell_my_uid() failed with [%d] %s",
		          errnum, strerror(errnum));
	if (!pathname || !(*pathname))
	{
		exp_return = EINVAL;
	}
	else if (false == is_path_there(pathname))
	{
		exp_return = ENOENT;
	}
	else if (0 != my_uid)
	{
		path_owner = get_owner(pathname, &errnum);
		ck_assert_msg(0 == errnum, "get_owner(%s) failed with [%d] %s", pathname,
			          errnum, strerror(errnum));
		// From chown(2)... Only a privileged process may change the UID of a file: UID 0
		if (CSSO_SKIP_UID != exp_uid)
		{
			exp_return = EPERM;  // Permission error
		}
		// From chown(2)... A privileged process may change the GID arbitrarily: UID 0
		else if (CSSO_SKIP_GID != exp_gid)
		{
			// The UID of a file may change the file's GID to any GID of which that UID is a member.
			if (my_uid == path_owner)
			{
				// Is my UID a member of the exp_gid GID?
				if (false == compare_compatibility(my_uid, exp_gid))
				{
					exp_return = EPERM;
				}
			}
		}
	}
	// TO DO: DON'T DO NOW... Implement devops code to check CAP_CHOWN capability because...
	// From chown(2)...
	// Only a privileged process may change the UID of a file: CAP_CHOWN capability
	// A privileged process may change the GID arbitrarily: CAP_CHOWN capability

	// DONE
	return exp_return;
}


gid_t get_new_gid(void)
{
	// LOCAL VARIABLES
	int errnum = 0;             // Errno values
	gid_t new_gid = 0;          // New GID for a test case
	gid_t *compat_gids = NULL;  // Array of compatible GIDs
	bool got_one = false;       // Flow control variable

	// PRIORITIES
	// 1. CSSO_DEF_GID
#ifdef CSSO_DEF_GID
	new_gid = CSSO_DEF_GID;  // Use the test-runner-determined default GID
	got_one = true;  // We got one!
#endif  /* CSSO_DEF_GID */
	// 2. get_shell_compatible_gid()
	if (false == got_one)
	{
		compat_gids = get_shell_compatible_gid(&errnum);
		ck_assert_msg(0 == errnum, "get_shell_compatible_gid() failed with [%d] %s",
			          errnum, strerror(errnum));
		ck_assert_msg(NULL != compat_gids, "get_shell_compatible_gid() silently failed with NULL");
		if (compat_gids)
		{
			new_gid = compat_gids[0];  // There's at least one so just use the first one
			got_one = true;  // We got one!
		}
	}
	// 3. My GID
	if (false == got_one)
	{
		new_gid = get_shell_my_gid(&errnum);  // Just use my GID
		ck_assert_msg(0 == errnum, "get_shell_my_gid() failed with [%d] %s",
			          errnum, strerror(errnum));
		if (!errnum)
		{
			got_one = true;  // We finally got one!
		}
	}

	// DONE
	if (compat_gids)
	{
		errnum = free_devops_mem((void **)&compat_gids);
		ck_assert_msg(0 == errnum, "free_devops_mem(&compatible_GID_array) failed with [%d] %s",
			          errnum, strerror(errnum));
	}
	ck_assert_msg(true == got_one, "The call to get_new_gid() failed to produce a GID");
	return new_gid;
}


void reset_global_ownership(bool take_over)
{
	// LOCAL VARIABLES
	int errnum = CANARY_INT;  // Errno values
	uid_t tmp_uid = 0;		  // Temporary UID variable
	gid_t tmp_gid = 0;		  // Temporary GID variable
	bool follow_sym = false;  // No need because we'll also be resetting the file it points to
	// Array of the global pointers to iterate over
	testPathDetails_ptr test_structs[] = {test_dir_details, test_file_details, test_pipe_details,
										  test_socket_details, test_symlink_details};

	// SETUP
	if (true == take_over)
	{
		// Get UID
		tmp_uid = get_shell_my_uid(&errnum);
		ck_assert_msg(0 == errnum, "get_shell_my_uid() failed with [%d] %s",
					  errnum, strerror(errnum));
		// Get GID
		tmp_gid = get_shell_my_gid(&errnum);
		ck_assert_msg(0 == errnum, "get_shell_my_gid() failed with [%d] %s",
					  errnum, strerror(errnum));
	}

	// RESET OWNERSHIP
	for (int i = 0; i < (sizeof(test_structs) / sizeof(test_structs[0])); i++)
	{
		if (test_structs[i])
		{
			if (false == take_over)
			{
				tmp_uid = test_structs[i]->orig_owner;
				tmp_gid = test_structs[i]->orig_group;
			}
			FPRINTF_ERR("reset_global_ownership() IS SETTING %s OWNERSHIP TO UID %u AND GID %u\n", test_structs[i]->pathname, tmp_uid, tmp_gid);  // DEBUGGING
			// Reset owner
			errnum = set_owner_id(test_structs[i]->pathname, tmp_uid, follow_sym);
			if (EPERM == errnum)
			{
				FPRINTF_ERR("%s You may need to reset the owner of '%s' to %u\n",
					        DEBUG_WARNG_STR, test_structs[i]->pathname, tmp_uid);
			}
			else
			{
				ck_assert_msg(0 == errnum, "set_owner_id(%s, %u) failed with [%d] %s",
					          test_structs[i]->pathname, tmp_uid, errnum, strerror(errnum));
			}
			// Reset group
			errnum = set_group_id(test_structs[i]->pathname, tmp_gid, follow_sym);
			if (EPERM == errnum)
			{
				FPRINTF_ERR("%s You may need to reset the group of '%s' to %u\n",
					        DEBUG_WARNG_STR, test_structs[i]->pathname, tmp_gid);
			}
			else
			{
				ck_assert_msg(0 == errnum, "set_group_id(%s, %u) failed with [%d] %s",
					          test_structs[i]->pathname, tmp_gid, errnum, strerror(errnum));
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


void run_test_case(const char *pathname, const char *target_name, bool follow_sym,
				   uid_t new_uid, gid_t new_gid)
{
	// LOCAL VARIABLES
	int actual_ret = 0;               // Return value of the tested function
	int exp_return = 0;               // Expected return value for this test input
	uid_t exp_uid = new_uid;          // Expected UID
	gid_t exp_gid = new_gid;          // Expected GID
	// uid_t curr_uid = 0;               // Current UID
	// gid_t curr_gid = 0;               // Current GID
	const char *id_check = pathname;  // Filename to check *IDs for

	// SETUP
	// Should we use target_name instead?
	if (pathname && *pathname && true == is_path_there(pathname))
	{
		if (true == follow_sym)
		{
			if (true == is_sym_link(pathname, &actual_ret))
			{
				id_check = target_name;
			}
			else
			{
				ck_assert_msg(0 == actual_ret, "is_sym_link(%s) failed with [%d] %s",
							  pathname, actual_ret, strerror(actual_ret));
			}
		}
		actual_ret = CANARY_INT;  // Reset temp variable
		// Which IDs should we expect?
		if (CSSO_SKIP_UID == new_uid)
		{
			exp_uid = get_owner(id_check, &actual_ret);
			ck_assert_msg(0 == actual_ret, "get_owner(%s) failed with [%d] %s",
						  id_check, actual_ret, strerror(actual_ret));
		}
		if (CSSO_SKIP_GID == new_gid)
		{
			exp_gid = get_group(id_check, &actual_ret);
			ck_assert_msg(0 == actual_ret, "get_group(%s) failed with [%d] %s",
						  id_check, actual_ret, strerror(actual_ret));
		}
	}

	// RUN IT
	// 1. Determine expected results
	exp_return = determine_exp_return(id_check, new_uid, new_gid);
	// 2. Call the function
	actual_ret = set_ownership(pathname, new_uid, new_gid, follow_sym);
	// 3. Compare actual results to expected results
	ck_assert_msg(exp_return == actual_ret, "set_ownership(%s, %u, %u) returned [%d] '%s' "
				  "instead of [%d] '%s", pathname, new_uid, new_gid, actual_ret,
				  strerror(actual_ret), exp_return, strerror(exp_return));
	// No need to compare ownership if the expected result is "error"
	if (0 == exp_return)
	{
		compare_ownership(id_check, exp_uid, exp_gid);
	}
	FPRINTF_ERR("set_ownership(%s, %u, %u) returned [%d] '%s'",
		        pathname, new_uid, new_gid, actual_ret, strerror(actual_ret));  // DEBUGGING

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
		errnum = setup_struct_owners(test_pipe_details, true);
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
		errnum = setup_struct_owners(test_socket_details, true);
		ck_assert_msg(0 == errnum, "setup_struct_owners(%s struct) failed with [%d] %s",
					  test_socket_details->pathname, errnum, strerror(errnum));
	}
	else
	{
		ck_abort_msg("The setup() test fixture did not make the socket");
	}

	// RESET OWNERSHIP
	reset_global_ownership(true);

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
		errnum = setup_struct_owners(temp_struct, false);
		if (ENOENT == errnum)
		{
			errnum = 0;  // Ignore "file/dir missing" errors
		}
		ck_assert_msg(0 == errnum, "setup_struct_owners(%s struct) failed with [%d] %s",
					  temp_pathname, errnum, strerror(errnum));
	}

	// DONE
	if (errnum && temp_struct)
	{
		teardown_struct(&temp_struct);  // Something failed so undo it
	}
	return temp_struct;
}


int setup_struct_owners(testPathDetails_ptr new_struct, bool must_succeed)
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

	// SETUP OWNERS
	// orig_owner
	if (!errnum)
	{
		new_struct->orig_owner = get_owner(temp_pathname, &errnum);
		if (true == must_succeed)
		{
			ck_assert_msg(0 == errnum, "get_owner(%s) failed with [%d] %s",
						  temp_pathname, errnum, strerror(errnum));
		}
	}
	// orig_group
	if (!errnum)
	{
		new_struct->orig_group = get_group(temp_pathname, &errnum);
		if (true == must_succeed)
		{
			ck_assert_msg(0 == errnum, "get_group(%s) failed with [%d] %s",
						  temp_pathname, errnum, strerror(errnum));
		}
	}

	// DONE
	return errnum;
}


void teardown(void)
{
	// RESET OWNERSHIP
	reset_global_ownership(false);

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


void teardown_struct(testPathDetails_ptr* old_struct)
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


START_TEST(test_n01_directory)
{
	// LOCAL VARIABLES
	bool follow = true;             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;  // Test case input
	gid_t new_gid = get_new_gid();  // Test case input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_dir_details->pathname;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


START_TEST(test_n02_named_pipe)
{
	// LOCAL VARIABLES
	bool follow = true;             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;  // Test case input
	gid_t new_gid = get_new_gid();  // Test case input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_pipe_details->pathname;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


START_TEST(test_n03_regular_file)
{
	// LOCAL VARIABLES
	bool follow = true;             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;  // Test case input
	gid_t new_gid = get_new_gid();  // Test case input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_file_details->pathname;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


START_TEST(test_n04_socket)
{
	// LOCAL VARIABLES
	bool follow = true;             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;  // Test case input
	gid_t new_gid = get_new_gid();  // Test case input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_socket_details->pathname;

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


START_TEST(test_n05_symbolic_link_follow)
{
	// LOCAL VARIABLES
	bool follow = true;             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;  // Test case input
	gid_t new_gid = get_new_gid();  // Test case input
	// Absolute path for test input as resolved against the repo name
	char *input_abs_path = test_symlink_details->pathname;
	// Absolute path for actual_rel_path as resolved against the repo name
	char *actual_abs_path = test_file_details->pathname;

	// RUN TEST
	run_test_case(input_abs_path, actual_abs_path, follow, new_uid, new_gid);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/


START_TEST(test_e01_null_filename)
{
	// LOCAL VARIABLES
	bool follow = true;             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;  // Test case input
	gid_t new_gid = get_new_gid();  // Test case input
	char *input_abs_path = NULL;    // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


START_TEST(test_e02_empty_filename)
{
	// LOCAL VARIABLES
	bool follow = true;                          // Test case input
	uid_t new_uid = CSSO_SKIP_UID;               // Test case input
	gid_t new_gid = get_new_gid();               // Test case input
	char input_abs_path[] = { "\0 NOT HERE!" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


START_TEST(test_e03_missing_filename)
{
	// LOCAL VARIABLES
	bool follow = true;                             // Test case input
	uid_t new_uid = CSSO_SKIP_UID;                  // Test case input
	gid_t new_gid = get_new_gid();                  // Test case input
	char input_abs_path[] = { "/file/not/found" };  // Test case input

	// RUN TEST
	run_test_case(input_abs_path, NULL, follow, new_uid, new_gid);
}
END_TEST


// /**************************************************************************************************/
// /************************************** BOUNDARY TEST CASES ***************************************/
// /**************************************************************************************************/
// START_TEST(test_b01_sec_min_value)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = INT_MIN + 1;           // Seconds test input
// 	long new_nsec = 0x7E57;                 // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b02_sec_almost_epoch)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = -1;                    // Seconds test input
// 	long new_nsec = 0x7E57;                 // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b03_sec_epoch_time)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = 0;                     // Seconds test input
// 	long new_nsec = 0x7E57;                 // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b04_sec_barely_past_epoch)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = 1;                     // Seconds test input
// 	long new_nsec = 0x7E57;                 // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b05_sec_static_now)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = 0x66197EA4;            // Secs test input: 4/12/2024, 1:34:12 PM (1712946852)
// 	long new_nsec = 0x7E57;                 // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b06_sec_max_value)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = INT_MAX;               // Seconds test input
// 	long new_nsec = 0x7E57;                 // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b07_nsec_low_very_bad)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = EINVAL;                // Expected results
// 	time_t new_sec = 0x7E57;                // Seconds test input
// 	long new_nsec = LONG_MIN;               // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b08_nsec_low_barely_bad)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = EINVAL;                // Expected results
// 	time_t new_sec = 0x7E57;                // Seconds test input
// 	long new_nsec = UTIME_NSEC_MIN - 1l;    // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b09_nsec_low_barely_good)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = 0x7E57;                // Seconds test input
// 	long new_nsec = UTIME_NSEC_MIN;         // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b10_nsec_high_barely_good)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = 0x7E57;                // Seconds test input
// 	long new_nsec = UTIME_NSEC_MAX;         // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b11_nsec_high_barely_bad)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = EINVAL;                // Expected results
// 	time_t new_sec = 0x7E57;                // Seconds test input
// 	long new_nsec = UTIME_NSEC_MAX + 1l;    // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_b12_nsec_high_very_bad)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = EINVAL;                // Expected results
// 	time_t new_sec = 0x7E57;                // Seconds test input
// 	long new_nsec = LONG_MAX;               // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// /**************************************************************************************************/
// /*************************************** SPECIAL TEST CASES ***************************************/
// /**************************************************************************************************/
// START_TEST(test_s01_missing_filename)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                                 // Follow symlinks
// 	int exp_result = ENOENT;                            // Expected results
// 	time_t new_sec = 0x7E57;                            // Seconds test input
// 	long new_nsec = 0xC0DE;                             // Nanoseconds test input
// 	char input_abs_path[] = { "/does/not/exist.txt" };  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_s02_symbolic_link_nofollow)
// {
// 	// LOCAL VARIABLES
// 	bool follow = false;        // Follow symlinks
// 	int exp_result = 0;         // Expected results
// 	time_t new_sec = 0x4B1D;    // Seconds test input
// 	long new_nsec = 0xB16C0DE;  // Nanoseconds test input
// 	// Absolute path for test input as resolved against the repo name
// 	char *input_abs_path = test_sym_link;
// 	// Absolute path for actual_rel_path as resolved against the repo name
// 	char *actual_abs_path = test_file_path;

// 	// RUN TEST
// 	run_test_case(input_abs_path, actual_abs_path, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST
 

// START_TEST(test_s03_block_device)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                        // Follow symlinks
// 	int exp_result = EPERM;                    // Expected results
// 	time_t new_sec = 0x7E57;                   // Seconds test input
// 	long new_nsec = 0xC0DE;                    // Nanoseconds test input
// 	char input_abs_path[] = { "/dev/loop0" };  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// START_TEST(test_s04_character_device)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                       // Follow symlinks
// 	int exp_result = EPERM;                   // Expected results
// 	time_t new_sec = 0x7E57;                  // Seconds test input
// 	long new_nsec = 0xC0DE;                   // Nanoseconds test input
// 	char input_abs_path[] = { "/dev/null" };  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


// /*
//  *	NOTE: If the seconds are set to INT_MIN, the nanoseconds appear to always be set to 0.
//  */
// START_TEST(test_s05_sec_min_value_edge_case)
// {
// 	// LOCAL VARIABLES
// 	bool follow = true;                     // Follow symlinks
// 	int exp_result = 0;                     // Expected results
// 	time_t new_sec = INT_MIN;               // Seconds test input
// 	long new_nsec = 0;                      // Nanoseconds test input
// 	char *input_abs_path = test_file_path;  // Test case input

// 	// RUN TEST
// 	run_test_case(input_abs_path, NULL, follow, exp_result, new_sec, new_nsec);
// }
// END_TEST


Suite *set_ownership_suite(void)
{
	// LOCAL VARIABLES
	Suite *suite = suite_create("SFMW_Set_Ownership");  // Test suite
	TCase *tc_normal = tcase_create("Normal");       // Normal test cases
	TCase *tc_error = tcase_create("Error");         // Error test cases
	// TCase *tc_boundary = tcase_create("Boundary");   // Boundary test cases
	// TCase *tc_special = tcase_create("Special");     // Special test cases

	// SETUP TEST CASES
	tcase_add_checked_fixture(tc_normal, setup, teardown);
	tcase_add_checked_fixture(tc_error, setup, teardown);
	// tcase_add_checked_fixture(tc_boundary, setup, teardown);
	// tcase_add_checked_fixture(tc_special, setup, teardown);
	tcase_add_test(tc_normal, test_n01_directory);
	tcase_add_test(tc_normal, test_n02_named_pipe);
	tcase_add_test(tc_normal, test_n03_regular_file);
	tcase_add_test(tc_normal, test_n04_socket);
	tcase_add_test(tc_normal, test_n05_symbolic_link_follow);
	tcase_add_test(tc_error, test_e01_null_filename);
	tcase_add_test(tc_error, test_e02_empty_filename);
	tcase_add_test(tc_error, test_e03_missing_filename);
	// tcase_add_test(tc_boundary, test_b01_sec_min_value);
	// tcase_add_test(tc_boundary, test_b02_sec_almost_epoch);
	// tcase_add_test(tc_boundary, test_b03_sec_epoch_time);
	// tcase_add_test(tc_boundary, test_b04_sec_barely_past_epoch);
	// tcase_add_test(tc_boundary, test_b05_sec_static_now);
	// tcase_add_test(tc_boundary, test_b06_sec_max_value);
	// tcase_add_test(tc_boundary, test_b07_nsec_low_very_bad);
	// tcase_add_test(tc_boundary, test_b08_nsec_low_barely_bad);
	// tcase_add_test(tc_boundary, test_b09_nsec_low_barely_good);
	// tcase_add_test(tc_boundary, test_b10_nsec_high_barely_good);
	// tcase_add_test(tc_boundary, test_b11_nsec_high_barely_bad);
	// tcase_add_test(tc_boundary, test_b12_nsec_high_very_bad);
	// tcase_add_test(tc_special, test_s01_missing_filename);
	// tcase_add_test(tc_special, test_s02_symbolic_link_nofollow);
	// tcase_add_test(tc_special, test_s03_block_device);
	// tcase_add_test(tc_special, test_s04_character_device);
	// tcase_add_test(tc_special, test_s05_sec_min_value_edge_case);
	suite_add_tcase(suite, tc_normal);
	suite_add_tcase(suite, tc_error);
	// suite_add_tcase(suite, tc_boundary);
	// suite_add_tcase(suite, tc_special);

	return suite;
}


int main(void)
{
	// LOCAL VARIABLES
	int errnum = 0;  // Errno from the function call
	// Relative path for this test case's input
	char log_rel_path[] = { "./code/test/test_output/check_sfmw_set_ownership.log" };
	// Absolute path for log_rel_path as resolved against the repo name
	char *log_abs_path = resolve_to_repo(SKID_REPO_NAME, log_rel_path, false, &errnum);
	int number_failed = 0;
	Suite *suite = NULL;
	SRunner *suite_runner = NULL;

	// SETUP
	suite = set_ownership_suite();
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
