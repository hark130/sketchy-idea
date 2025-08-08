#!/bin/bash

# This script was made to help automate exporting 13-4's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)

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
    printf "Command output:\n"
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
    run_manual_test_command "$FULL_CMD"
    EXIT_CODE=$?

    # DONE
    return $EXIT_CODE
}

CWD=`pwd`                                                      # Current working directory
TEST_SOURCE_DIR="$CWD/code/test/test_input/"                   # Input directory
TEST_SOURCE_FILE="$CWD/code/test/test_input/regular_file.txt"  # Input file
TEST_DEST_SYM_DIR="./code/test/test_output/sym_dir"            # Destination dir symbolic link
TEST_DEST_SYM_FILE="./code/test/test_output/sym_file"          # Destination file symbolic link
TEST_DEST_HARD_FILE="./code/test/test_output/hard_file"        # Destination file hard link
BOOKEND="***"                                                  # Formatting
TEMP_RET=0                                                     # Temporary exit code var

# 1. Stores the date
date && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. Symbolic Links
# 5.A.i. Pre-Cleanup
rm --force $TEST_DEST_SYM_DIR  # Remove pre-existing symbolic link
rm --force $TEST_DEST_SYM_FILE  # Remove pre-existing symbolic link
# 5.A.ii. Manual Test Code
# Create symbolic link (dir)
printf "%s Create Symbolic Link (dir) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command_verbose "./code/dist/test_sfl_create_sym_link.bin $TEST_SOURCE_DIR $TEST_DEST_SYM_DIR"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    run_manual_test_command "ls -l $TEST_DEST_SYM_DIR"
    run_manual_test_command "ls $TEST_DEST_SYM_DIR"
else
    echo "Exited with: $TEMP_RET"
fi
# Create symbolic link (file)
printf "%s Create Symbolic Link (file) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command_verbose "./code/dist/test_sfl_create_sym_link.bin $TEST_SOURCE_FILE $TEST_DEST_SYM_FILE"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    run_manual_test_command "ls -l $TEST_DEST_SYM_FILE"
    run_manual_test_command "head -n 4 $TEST_DEST_SYM_FILE"
else
    echo "Exited with: $TEMP_RET"
fi
# 5.A.iii. Post-Cleanup
rm --force $TEST_DEST_SYM_DIR  # Remove symbolic link
rm --force $TEST_DEST_SYM_FILE  # Remove symbolic link

# 5.B. Hard Links
# 5.B.i. Pre-Cleanup
rm --force $TEST_DEST_HARD_FILE  # Remove pre-existing symbolic link
# 5.B.ii. Manual Test Code
# Create hard link (file)
printf "%s Create Hard Link (file) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command_verbose "./code/dist/test_sfl_create_hard_link.bin $TEST_SOURCE_FILE $TEST_DEST_HARD_FILE"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    printf "Compare inode numbers between '%s' and '%s'...\n" $TEST_SOURCE_FILE $TEST_DEST_HARD_FILE
    run_manual_test_command "ls -li $TEST_SOURCE_FILE"
    run_manual_test_command "ls -li $TEST_DEST_HARD_FILE"
else
    echo "Exited with: $TEMP_RET"
fi

# 5.B.iii. Post-Cleanup
rm --force $TEST_DEST_HARD_FILE  # Remove symbolic link
