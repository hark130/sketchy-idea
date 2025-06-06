#!/bin/bash

# This script was made to help automate exporting 2-11's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
#   A. Prove(?) the compiler used for each binary
#   B. Compares the size between both binaries
#   C. Compare some ELF complexity between both binaries (program/section header count)
#   D. Time trials

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
GLIBC_BUILD="./code/dist/test_misc_glibc_vs_musl-static_glibc.bin"  # Binary built with glibc
MUSL_BUILD="./code/dist/test_misc_glibc_vs_musl-static_musl.bin"    # Binary built with musl
GLIBC_PERF_DATA="./code/test/test_output/glibc_perf.data"           # Perf report: glibc
MUSL_PERF_DATA="./code/test/test_output/musl_perf.data"             # Perf report: musl
TEST_NUM=10000000                                                   # <RANGE_END>

# 1. Stores the date
date && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. Prove(?) the compiler used for each binary
printf "\n%s Each of the following binaries was statically compiled using a different library %s\n" "$BOOKEND" "$BOOKEND"
printf "%s was compiled with: %s\n" "$GLIBC_BUILD" "$(strings "$GLIBC_BUILD" | grep glibc.ld.so)"
printf "%s was compiled with: %s\n" "$MUSL_BUILD" "$(strings "$MUSL_BUILD" | grep musl-)"

# 5.B. Compares the size between both binaries
printf "\n%s The musl binary is significantly smaller than the glibc binary %s\n" "$BOOKEND" "$BOOKEND"
ls -lh $GLIBC_BUILD
ls -lh $MUSL_BUILD

# 5.C. Compare some ELF complexity between both binaries (program/section header count)
printf "\n%s The musl binary is less complex than the glibc binary %s\n" "$BOOKEND" "$BOOKEND"
printf "Compare the number of program headers:\n"
printf "%s - %s\n" "$GLIBC_BUILD" "$(readelf -l "$GLIBC_BUILD" | grep "There are" | awk -F", starting" '{print $1}')"
printf "%s - %s\n" "$MUSL_BUILD" "$(readelf -l "$MUSL_BUILD" | grep "There are" | awk -F", starting" '{print $1}')"
printf "Compare the number of section headers:\n"
printf "%s - %s\n" "$GLIBC_BUILD" "$(readelf -S "$GLIBC_BUILD" | grep "There are" | awk -F", starting" '{print $1}')"
printf "%s - %s\n" "$MUSL_BUILD" "$(readelf -S "$MUSL_BUILD" | grep "There are" | awk -F", starting" '{print $1}')"

# 5.D. Time trials
# SETUP
# sudo apt install linux-tools-generic linux-tools-`uname -r`
# cat /proc/sys/kernel/perf_event_paranoid  # 4
# sudo sysctl -w kernel.perf_event_paranoid=1  # kernel.perf_event_paranoid = 1
# cat /proc/sys/kernel/perf_event_paranoid  # 1
# DO IT
printf "\n%s Time Trials! %s\n" "$BOOKEND" "$BOOKEND"
printf "Compare the time elapsed:\n"
printf "%s - %s\n" "$GLIBC_BUILD" "$(perf stat $GLIBC_BUILD $TEST_NUM 2>&1 | grep "time elapsed")"
printf "%s - %s\n" "$MUSL_BUILD" "$(perf stat $MUSL_BUILD $TEST_NUM 2>&1 | grep "time elapsed")"
printf "Compare the peformance:\n"
run_manual_test_command "sudo perf record -o $GLIBC_PERF_DATA $GLIBC_BUILD $TEST_NUM > /dev/null 2>&1"
run_manual_test_command "sudo perf record -o $MUSL_PERF_DATA $MUSL_BUILD $TEST_NUM > /dev/null 2>&1"
run_manual_test_command "sudo perf diff $GLIBC_PERF_DATA $MUSL_PERF_DATA"
