#!/bin/bash

# This script was made to help automate exporting 13-2's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code)

#
# PURPOSE:
#	Run manual test code in a standardized way
# ARGUMENTS:
#   command: The command, with a variable number of argments, to echo and then execute
# RETURN:
#   The exit code from command execution
#
run_manual_test_command()
{
    # LOCAL VARIABLES
    FULL_CMD="$@"             # Full manual test command
    CMD_ARRAY=($FULL_CMD)     # Full manual test command as an array
    BASE_CMD=${CMD_ARRAY[0]}  # Base manual test binary
    EXIT_CODE=0               # Exit code from command execution

    # DO IT
    # Invoke Usage
    bash -c "$BASE_CMD"
    echo
    # Echo
    printf "The full command is '%s'\n" "$@"
    echo
    # Execute
    bash -c "$@"
    EXIT_CODE=$?
    # Vertical whitespace
    printf "\n\n"

    # DONE
    return $EXIT_CODE
}

TEST_FILE="code/test/test_input/regular_file.txt"
TEST_DIR="./code/test/test_input/"
BOOKEND="***"

# 1. Stores the date
date && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# Set Times (file)
printf "%s Set Times (file) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "code/dist/test_sfmw_set_times.bin $TEST_FILE 1712946852"
echo "Reset Times"
run_manual_test_command "code/dist/test_sfmw_set_times.bin $TEST_FILE `date +%s`"
# Set Times (dir)
printf "%s Set Times (dir) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "code/dist/test_sfmw_set_times.bin $TEST_DIR 1712946852"
echo "Reset Times"
run_manual_test_command "code/dist/test_sfmw_set_times.bin $TEST_DIR `date +%s`"
# Set Ownership (file)
printf "%s Set Ownership (file) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "./code/dist/test_sfmw_set_ownership.bin $TEST_FILE -1 `id -g pulse`"
echo "Reset Ownership"
run_manual_test_command "./code/dist/test_sfmw_set_ownership.bin $TEST_FILE `id -u` `id -g`"
# Set Ownership (dir)
printf "%s Set Ownership (dir) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "./code/dist/test_sfmw_set_ownership.bin $TEST_DIR -1 `id -g pulse`"
echo "Reset Ownership"
run_manual_test_command "./code/dist/test_sfmw_set_ownership.bin $TEST_DIR `id -u` `id -g`"
