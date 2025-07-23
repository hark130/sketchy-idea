#!/bin/bash

# This script was made to help automate exporting SKID output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Prints identifying information
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
# 5.A.


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

BOOKEND="***"                                                       # Formatting
TEMP_RET=0                                                          # Temporary exit code var
EXIT_CODE=0                                                         # Exit value

# 1. Stores the date
printf "\n%s This output was created by %s on %s %s\n" "$BOOKEND" "$(basename "$0")" "$(date)" "$BOOKEND" && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A.
echo
print_title "TO DO: DON'T DO NOW... Put something here!"

# DONE
if [ $EXIT_CODE -ne 0 ]
then
    echo "There appears to have been an error in the execution of this script: $EXIT_CODE"
fi
exit $EXIT_CODE
