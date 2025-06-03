#!/bin/bash

# This script was made to help automate exporting 2-4's output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Stores the date
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests (by running them again)
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
#   A. Calls the manual test code default build
#   B. Disassembles the manual test code with objdump
#   C. Compiles the manual test code with the USE_OBFUSCATED_FLOW macro defined
#   D. Calls the obfuscated manual test code
#   E. Disassembles the obfuscated manual test code with objdump

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

BOOKEND="***"                                                     # Formatting
TEMP_RET=0                                                        # Temporary exit code var
EXIT_CODE=0                                                       # Exit value
CLEAN_CODE="./code/dist/test_misc_setjmp_longjmp.bin"             # "Clean" build
OBFUS_CODE="./code/dist/test_misc_setjmp_longjmp_obfuscated.bin"  # "Obfuscated" build

# 1. Stores the date
date && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. Calls the manual test code default build
printf "%s Executing manual test code (clean) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command $CLEAN_CODE
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Manual test code (clean) exited with: $TEMP_RET"
fi

# 5.B. Disassembles the manual test code with objdump
printf "%s Disassemble manual test code (clean) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "objdump -d $CLEAN_CODE"
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Disassembly of the manual test code (clean) exited with: $TEMP_RET"
fi

# 5.C. Compiles the manual test code with the USE_OBFUSCATED_FLOW macro defined
printf "%s Compiling manual test code (obfuscated) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command  "gcc -I ./code/include/ -o $OBFUS_CODE ./code/test/test_misc_setjmp_longjmp.c -DUSE_OBFUSCATED_FLOW"
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Compilation of the manual test code (obfuscated) exited with: $TEMP_RET"
fi


# 5.D. Calls the obfuscated manual test code
printf "%s Executing manual test code (obfuscated) %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command $OBFUS_CODE
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Manual test code (obfuscated) exited with: $TEMP_RET"
fi

# 5.E. Disassembles the obfuscated manual test code with objdump
run_manual_test_command "objdump -d $OBFUS_CODE"
TEMP_RET=$?
if [ $TEMP_RET -ne 0 ]
then
    EXIT_CODE=$TEMP_RET
    echo "Disassembly of the manual test code (obfuscated) exited with: $TEMP_RET"
fi

# DONE
if [ $EXIT_CODE -ne 0 ]
then
    echo "There appears to have been an error in the execution of this script: $EXIT_CODE"
fi
exit $EXIT_CODE
