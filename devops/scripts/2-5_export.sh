#!/bin/bash

# This script was made to help automate exporting 2-5's output output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Prints identifying information
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
# 5.A. Setup
# 5.A.i     Install the shared object
# 5.E. Showcase the visibility attribute
# 5.E.i     Prove the manual test code works when statically compiled against source
# 5.E.ii    Prove the linker fails when utilizing the shared object
# 5.E.iii   Prove internal functions aren't dynamically exported
# 5.F. Showcase the cleanup attribute
# 5.F.i     Execute the code
# 5.F.ii    Also run it through Valgrind


BOOKEND="***"  # SPOT for formatting titles and banners


#
# PURPOSE:
#   Standardize how high-level banners are printed
#       *****************
#       *** The title ***
#       *****************
# ARGUMENTS:
#   title: The title to print as a string inside a banner
# RETURN:
#   The exit code
#
print_banner()
{
    # LOCAL VARIABLES
    EXIT_CODE=0                                                # Exit code from command execution
    FORMAT_CHAR="${BOOKEND:0:1}"                               # Formatting
    TITLE="$@"                                                 # The title argument
    LENGTH=$((${#BOOKEND} + 1 + ${#TITLE} + 1 + ${#BOOKEND}))  # Length of the title
    BANNER=$(printf "%${LENGTH}s" | tr " " "$FORMAT_CHAR")     # Banner string

    # DO IT
    printf "\n%s\n" "$BANNER"  # Header
    print_title "$TITLE"
    EXIT_CODE=$?
    printf "%s\n" "$BANNER"  # Footer

    # DONE
    return $EXIT_CODE
}

#
# PURPOSE:
#   Standardize how titles are printed in the exported output:
#   No leading newline will be added
#       *** The title ***
# ARGUMENTS:
#   title: The title to print as a string
# RETURN:
#   The exit code
#
print_title()
{
    # LOCAL VARIABLES
    EXIT_CODE=0    # Exit code from command execution
    TITLE="$@"     # The title argument

    # DO IT
    printf "%s %s %s\n" "$BOOKEND" "$TITLE" "$BOOKEND"
    EXIT_CODE=$?

    # DONE
    return $EXIT_CODE
}

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

TEMP_RET=0                                                          # Temporary exit code var
EXIT_CODE=0                                                         # Exit value
DIST_DIR=./code/dist/                                               # Distribution directory
LIB_NAME=libsketchyidea.so.1.0.0                                    # Basename of the shared object
# Manual test binary names for 5.F. Showcase the cleanup attribute
CLEAN_MAN_TEST_BIN=("${DIST_DIR}test_sm_raii_string_macro.bin" "${DIST_DIR}test_sm_raii_void_macro.bin")

# 1. Stores the date
printf "\n%s This output was created by %s on %s %s\n" "$BOOKEND" "$(basename "$0")" "$(date)" "$BOOKEND" && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# # 3. Executes the unit tests with Valgrind
# ./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# # 4. Counts the number of unit tests (by running them again)
# for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. Setup
print_banner "SETUP"
# 5.A.i     Install the shared object
print_title "Installing the SKETCHY IDEA (SKID) shared object"
make install
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "make install failed with: $TEMP_RET"
else
# 5.B. Showcase the constructor attribute
# 5.C. Showcase the destructor attribute
# 5.D. Showcase the packed attribute
# 5.E. Showcase the visibility attribute
print_banner "GCC ATTRIBUTE: visibility"
# 5.E.i     Prove the manual test code works when statically compiled against source
print_title "Manual test code works when compiled against source"
run_manual_test_command "./code/dist/test_misc_sfmr_call_stat.bin ./code/test/test_input/regular_file.txt"
run_manual_test_command "./code/dist/test_misc_sfmr_call_stat.bin ./code/test/test_input/"
# 5.E.ii    Prove the linker fails when utilizing the shared object
run_manual_test_command "gcc -o ./code/dist/test_libskid_sfmr_call_stat.bin ./code/test/test_misc_sfmr_call_stat.c -lsketchyidea"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    EXIT_CODE=1
    echo "The compiler incorrectly succeeded at linking the test code to a hidden internal symbol"
else
    EXIT_CODE=0
    TEMP_RET=0  # Everything is actually fine
    echo "The compiler correctly failed to link against a hidden internal symbol"
# 5.E.iii   Prove internal functions aren't dynamically exported
print_title "The manual test code attempted to access a non-dynamic symbol in the shared object"
run_manual_test_command "nm -D $DIST_DIR$LIB_NAME | grep call_stat"
TEMP_RET=$?
if [ $TEMP_RET -eq 0 ]
then
    EXIT_CODE=1
    echo "The hidden internal symbol seems to have been exposed after all"
else
    EXIT_CODE=0
    TEMP_RET=0  # Everything is actually fine
    echo "The hidden internal symbol is not listed among the dynamic symbols exported from the shared object"
# 5.F. Showcase the cleanup attribute
print_banner "GCC ATTRIBUTE: cleanup"
print_title "SKID_AUTO_FREE_CHAR"
# 5.F.i     Execute the code (string)
run_manual_test_command "${CLEAN_MAN_TEST_BIN[0]} ${LIB_NAME}"
# 5.F.ii    Also run it through Valgrind (string)
run_manual_test_command "valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ${CLEAN_MAN_TEST_BIN[0]} ${LIB_NAME}"
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Valgrind unexpectedly with: $TEMP_RET"
else
print_title "SKID_AUTO_FREE_VOID"
# 5.F.i     Execute the code (void)
run_manual_test_command "${CLEAN_MAN_TEST_BIN[1]} 00000090"
# 5.F.ii    Also run it through Valgrind (void)
run_manual_test_command "valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ${CLEAN_MAN_TEST_BIN[1]} -0318"
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Valgrind unexpectedly with: $TEMP_RET"
fi
fi
fi
fi
fi

# DONE
if [ $EXIT_CODE -ne 0 ]
then
    echo "There appears to have been an error in the execution of this script: $EXIT_CODE"
fi
exit $EXIT_CODE
