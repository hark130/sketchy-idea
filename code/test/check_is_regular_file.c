/*
 *  Check unit test suit for skip_file_metadata_read.h's is_regular_file() function.
 */

#include <stdlib.h>
#include <check.h>                    // START_TEST(), END_TEST
// Local includes
#include "skip_file_metadata_read.h"  // is_character_device()


// Use this to help highlight an errnum that wasn't updated
#define CANARY_INT (int)0xBADC0DE


START_TEST(test_n01_regular_file)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_regular_file("./code/test/test_input/regular_file.txt", &errnum);
    ck_assert(true == result);  // This input is a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST


START_TEST(test_n02_directory)
{
    bool result = false;      // Return value from function call
    int errnum = CANARY_INT;  // Errno from the function call
    result = is_regular_file("./code/test/test_input/", &errnum);
    ck_assert(false == result);  // This input is *NOT* a regular file
    ck_assert_int_eq(0, errnum);  // The out param should be zeroized on success
}
END_TEST

 
Suite *is_regular_file_suite(void)
{
    Suite *suite = NULL;
    TCase *tc_core = NULL;

    suite = suite_create("Is_Regular_File");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_n01_regular_file);
    tcase_add_test(tc_core, test_n02_directory);
    suite_add_tcase(suite, tc_core);

    return suite;
}


int main(void)
{
    int number_failed = 0;
    Suite *suite = NULL;
    SRunner *suite_runner = NULL;

    suite = is_regular_file_suite();
    suite_runner = srunner_create(suite);

    srunner_run_all(suite_runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
