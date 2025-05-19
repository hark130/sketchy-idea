#!/bin/bash

# This script was made to help automate exporting 2-13's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
#   A. ltrace
#   B. strace

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

CWD=`pwd`                                                # Current working directory
TEST_INPUT_DIR="$CWD/code/test/test_input"               # Input directory
TEST_OUTPUT_DIR="$CWD/code/test/test_output"             # Output directory
LTRACE_OUTPUT_FILE="$TEST_OUTPUT_DIR/ltrace_output.txt"  # ltrace output
STRACE_OUTPUT_FILE="$TEST_OUTPUT_DIR/strace_output.txt"  # strace output
BOOKEND="***"                                            # Formatting
TEMP_RET=0                                               # Temporary exit code var

# 1. Stores the date
date && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. ltrace
printf "%s Utilizing ltrace %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "ltrace --output $LTRACE_OUTPUT_FILE ./code/dist/test_sdo_read_dir_contents.bin $TEST_INPUT_DIR"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    echo -e "\n$LTRACE_OUTPUT_FILE OUTPUT:\n"
    run_manual_test_command "cat $LTRACE_OUTPUT_FILE"
else
    echo "Exited with: $TEMP_RET"
fi

# 5.B. strace
printf "%s Utilizing strace %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "strace --output=$STRACE_OUTPUT_FILE ./code/dist/test_sdo_read_dir_contents.bin $TEST_INPUT_DIR"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    echo -e "\n$STRACE_OUTPUT_FILE OUTPUT:\n"
    run_manual_test_command "cat $STRACE_OUTPUT_FILE"
else
    echo "Exited with: $TEMP_RET"
fi
