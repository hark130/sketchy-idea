/*
 *  Check unit test suit for skid_file_metadata_read.h's is_character_device() function.
 *
 *  Copy/paste the following from the repo's top-level directory...

make -C code dist/check_sfmr_is_character_device.bin
code/dist/check_sfmr_is_character_device.bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all code/dist/check_sfmr_is_character_device.bin

 *
 */

#include <check.h>                    // START_TEST(), END_TEST
#include <limits.h>                   // PATH_MAX
#include <stdio.h>                    // printf()
#include <stdlib.h>
#include <unistd.h>                   // get_current_dir_name()
// Local includes
#include "devops_code.h"              // resolve_to_repo(), SKID_REPO_NAME
#include "skid_file_metadata_read.h"  // is_character_device()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE  // Actually, a reverse canary value


/**************************************************************************************************/
/*************************************** NORMAL TEST CASES ****************************************/
/**************************************************************************************************/
START_TEST(test_n01_block_device)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_character_device("/dev/loop0", &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n02_character_device)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_character_device("/dev/null", &errnum);
    ck_assert(true == result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n03_directory)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_character_device(input_abs_path, &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n04_named_pipe)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/named_pipe" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, false, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    remove_a_file(input_abs_path, true);  // Remove leftovers and ignore errors
    errnum = make_a_pipe(input_abs_path);
    ck_assert_msg(0 == errnum, "make_a_pipe(%s) failed with [%d] %s", input_abs_path,
                  errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_character_device(input_abs_path, &errnum);
    ck_assert(false == result);  // This input is a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    remove_a_file(input_abs_path, true);
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n05_regular_file)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/regular_file.txt" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_character_device(input_abs_path, &errnum);
    ck_assert(false == result);  // This input is a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


START_TEST(test_n06_socket)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_character_device("/var/run/dbus/system_bus_socket", &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n07_symbolic_link)
{
    // LOCAL VARIABLES
    const char *repo_name = SKID_REPO_NAME;  // Repo name
    bool result = false;                     // Return value from function call
    int errnum = CANARY_INT;                 // Errno from the function calls
    // Relative path for this test case's input
    char input_rel_path[] = { "./code/test/test_input/sym_link.txt" };
    // Absolute path for input_rel_path as resolved against the repo name
    char *input_abs_path = resolve_to_repo(repo_name, input_rel_path, true, &errnum);

    // VALIDATION
    // It is important resolve_to_repo() succeeds
    ck_assert_msg(0 == errnum, "resolve_to_repo(%s, %s) failed with [%d] %s", repo_name,
                  input_rel_path, errnum, strerror(errnum));
    errnum = CANARY_INT;  // Reset this temp var

    // TEST START
    result = is_character_device(input_abs_path, &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success

    // CLEANUP
    free_devops_mem(&input_abs_path);
}
END_TEST


/**************************************************************************************************/
/**************************************** ERROR TEST CASES ****************************************/
/**************************************************************************************************/
START_TEST(test_e01_null_filename)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_character_device(NULL, &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e02_empty_filename)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_character_device("\0 NOT HERE!", &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(EINVAL, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_e03_null_errnum)
{
    bool result = false;  // Return value from function call
    result = is_character_device("/dev/loop0", NULL);
    ck_assert(false == result);
}
END_TEST


/**************************************************************************************************/
/*************************************** SPECIAL TEST CASES ***************************************/
/**************************************************************************************************/
START_TEST(test_s01_missing_filename)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_character_device("/does/not/exist.txt", &errnum);
    ck_assert(false == result);
    ck_assert_int_eq(ENOENT, errnum);  // The out param should be zeroized on success
}
END_TEST

 
Suite *is_character_device_suite(void)
{
    Suite *suite = NULL;
    TCase *tc_core = NULL;

    suite = suite_create("SFMR_Is_Character_Device");

    /* Core test case */
    tc_core = tcase_create("Core");

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
    int number_failed = 0;
    Suite *suite = NULL;
    SRunner *suite_runner = NULL;

    suite = is_character_device_suite();
    suite_runner = srunner_create(suite);

    srunner_run_all(suite_runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
