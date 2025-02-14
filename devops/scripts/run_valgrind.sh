#!/bin/bash

# This script was made to help standardize how Valgrind is used to validate the Check unit tests.
# This script will "discover" ./code/dist/check_*.bin files and run each through Valgrind.
# Ensure all Check unit test binaries are built because this script will *not* invoke the build
# system.
#
# EXIT CODE
#   0 on success
#   1 if Valgrind found something


TEMP_RET=0                                                     # Temporary exit code var

# Executes the unit tests with Valgrind
for check_bin in $(ls code/dist/check_*.bin)
do
    CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $check_bin
    TEMP_RET=$?
    if [[ $TEMP_RET -ne 0 ]]
    then
        echo -e "\nValgrind appears to have found an issue with $check_bin!\n"
        break  # Valgrind found something
    fi
done

exit $TEMP_RET
