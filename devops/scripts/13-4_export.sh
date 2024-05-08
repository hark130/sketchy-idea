#!/bin/bash

# This script was made to help automate exporting 13-4's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code)

#
# PURPOSE:
#   Run manual test code in a standardized way
# ARGUMENTS:
#   command: The command, with a variable number of argments, to echo and then execute
# RETURN:
#   The exit code from command execution
#
run_manual_test_command()
{
    # LOCAL VARIABLES
    EXIT_CODE=0  # Exit code from command execution

    # DO IT
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

#
# PURPOSE:
#   Verbosely run manual test code in a standardized way (calls run_manual_test_command())
# ARGUMENTS:
#   command: The command, with a variable number of argments, to print usage for, echo and then
#       execute
# RETURN:
#   The exit code from command execution
#
run_manual_test_command_verbose()
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
    # Call run_manual_test_command()
    run_manual_test_command $FULL_CMD
    EXIT_CODE=$?

    # DONE
    return $EXIT_CODE
}

CWD=`pwd`                                                      # Current working directory
TEST_SOURCE_DIR="$CWD/code/test/test_input/"                   # Input directory
TEST_SOURCE_FILE="$CWD/code/test/test_input/regular_file.txt"  # Input file
TEST_DEST_SYM_DIR="./code/test/test_output/sym_dir"            # Destination dir symbolic link
TEST_DEST_SYM_FILE="./code/test/test_output/sym_file"          # Destination file symbolic link
BOOKEND="***"                                                  # Formatting
TEMP_RET=0                                                     # Temporary exit code var

# # 1. Stores the date
# date && \
# # 2. Runs the build system (which also executes the Check-based unit tests)
# make && echo && \
# # 3. Executes the unit tests with Valgrind
# for check_bin in $(ls code/dist/check_*.bin); do CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $check_bin; [[ $? -ne 0 ]] && break || continue; done && echo && \
# # 4. Counts the number of unit tests (by running them again)
# for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# A. Pre-Cleanup
rm --force $TEST_DEST_SYM_DIR  # Remove pre-existing symbolic link
rm --force $TEST_DEST_SYM_FILE  # Remove pre-existing symbolic link
# B. Manual Test Code
# Create symbolic link (dir)
run_manual_test_command_verbose "./code/dist/test_sfl_create_sym_link.bin $TEST_SOURCE_DIR $TEST_DEST_SYM_DIR"
TEMP_RET = $?
if [ $TEMP_RET -eq 0 ]
then
    run_manual_test_command "ls -l $TEST_DEST_SYM_DIR"
    run_manual_test_command "ls $TEST_DEST_SYM_DIR"
else
    echo "Exited with: $TEMP_RET"
fi
# Create symbolic link (file)
run_manual_test_command_verbose "./code/dist/test_sfl_create_sym_link.bin $TEST_SOURCE_FILE $TEST_DEST_SYM_FILE"
TEMP_RET = $?
if [ $TEMP_RET -eq 0 ]
then
    run_manual_test_command "ls -l $TEST_DEST_SYM_FILE"
    run_manual_test_command "ls $TEST_DEST_SYM_FILE"
else
    echo "Exited with: $TEMP_RET"
fi

# C. Post-Cleanup

