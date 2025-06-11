#!/bin/bash

# This script was made to help automate exporting 8-4's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
#   A. Showcases the "installed" shared object
#   B. Showcases select manual test code has been linked against the shared object
#   C. Executes that manual test code

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

BOOKEND="***"                                                       # Formatting
TEMP_RET=0                                                          # Temporary exit code var
EXIT_CODE=0                                                         # Exit value
DIST_DIR=./code/dist/
MAN_TEST_BIN=("${DIST_DIR}test_libskid_sa_read_cpu_tsc.bin" "${DIST_DIR}test_libskid_sdo_read_dir_contents.bin" "${DIST_DIR}test_libskid_sfmr_get_file_perms.bin")

# # 1. Stores the date
# date && \
# # 2. Runs the build system (which also executes the Check-based unit tests)
# make && echo && \
# # 3. Executes the unit tests with Valgrind
# ./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# # 4. Counts the number of unit tests (by running them again)
# for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. Showcases the "installed" shared object
printf "\n%s SKETCHY IDEA (SKID) has been released as a shared object and installed locally %s\n" "$BOOKEND" "$BOOKEND"
printf "Installed header files:\n"
run_manual_test_command "ls -l /usr/include/skid*.h"
printf "Installed shared object (with links):\n"
run_manual_test_command "ls -l /lib/*sketchyidea*"

# 5.B. Showcases select manual test code has been linked against the shared object
printf "\n%s Statically compiled manual test code was also linked against the shared object %s\n" "$BOOKEND" "$BOOKEND"
for linked_bin in `ls ${DIST_DIR}test_libskid_*.bin`; do
    static_bin=$(echo "$linked_bin" | sed 's/_libskid//')
    printf "%s was linked against the SKID shared object\n" "$linked_bin"
    run_manual_test_command "ldd ${linked_bin}"
    printf "%s was compiled against the source code\n" "$static_bin"
    run_manual_test_command "ldd ${static_bin}"
done

# 5.C. Executes that manual test code
printf "\n%s Executing the manual test code linked against the shared object %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "${MAN_TEST_BIN[0]}"
run_manual_test_command "${MAN_TEST_BIN[1]} ./"
run_manual_test_command "${MAN_TEST_BIN[2]} ./README.md"
