#!/bin/bash

# This script was made to help automate exporting 1-3's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code)
#

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

BOOKEND="***"  # SPOT for output formatting

# 1. Stores the date
date && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# Masking Signals
# Unmasking Signals
printf "%s Masking and Unmasking Signals %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "code/dist/test_ss_block_unblock.bin"
# Extended Signal Handlers
# 1. Sending integer data using signals
printf "%s Extended Signal Handler: Parent process reads integer data sent from a child process via signals %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "code/dist/test_ssh_handle_ext_read_queue_int.bin 10"
# 2. Translating signal codes
printf "%s Extended Signal Handler: Translating signal codes %s\n" "$BOOKEND" "$BOOKEND"
printf "Send the following command to speed this proccess up:\nkill -SIGUSR1 \`pidof test_ssh_handle_ext_signal_code.bin\`\n"
run_manual_test_command "code/dist/test_ssh_handle_ext_signal_code.bin"
# kill -s 10 `pidof test_ssh_handle_ext_signal_code.bin`
# 3. Identifying the sending process
printf "%s Extended Signal Handler: Identifying the sending process %s\n" "$BOOKEND" "$BOOKEND"
printf "Send the following command to speed this proccess up:\nkill -SIGUSR2 \`pidof test_ssh_handle_ext_sending_process.bin\`\n"
run_manual_test_command "code/dist/test_ssh_handle_ext_sending_process.bin"
# kill -s 12 `pidof test_ssh_handle_ext_sending_process.bin`
