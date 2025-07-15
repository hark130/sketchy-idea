#!/bin/bash

# This script was made to help automate exporting SKID output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Prints identifying information
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
# 5.A. fork
# 5.B. exec
# 5.C. exit
# 5.D. wait
# 5.E. clone


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
echo
# 5.A. fork
# 5.B. exec
# 5.C. exit
# 5.D. wait
print_banner "FORK && EXEC && WAIT && EXIT"
print_title "The redirect_bin_output.bin binary forks a child process..."
print_title "...that uses execvp() to execute command line arguments (after dup2()ing some fds)..."
print_title "...while the parent process waitpid()s on the child process to finish..."
print_title "...and the parent process calls exit()."
run_manual_test_command "ls -l *.txt  # No text files"
run_manual_test_command "./code/dist/redirect_bin_output.bin ls -l /home/vagrant/Repos/sketchy-idea  # No output"
run_manual_test_command "ls -l *.txt  # Redirected output from the above ls command"
run_manual_test_command "cat *ls-output.txt  # Redirected stdout from the ls command"
run_manual_test_command "cat *ls-errors.txt  # Redirected stderr from the ls command"
printf "NOTE: redirect_bin_output.bin was originally written to demonstrate dup() usage...\n"
# 5.E. clone
print_banner "CLONE"
print_title "The test_sc_sandbox_process.bin binary (optionally) utilizes clone()..."
print_title "...to sandbox a child process with a new PID namespace and..."
print_title "...a new UTS namespace."
run_manual_test_command "hostname  # Current hostname"
printf "The default behavior of the test_sc_sandbox_process.bin binary...\n"
run_manual_test_command "sudo ./code/dist/test_sc_sandbox_process.bin  # PIDs ID'd and hostname changed"
run_manual_test_command "hostname  # The hostname was modified"
printf "The --sandbox argument will clone() some namespaces...\n"
run_manual_test_command "sudo ./code/dist/test_sc_sandbox_process.bin --sandbox  # Running in new namespaces"
run_manual_test_command "hostname  # The hostname change did not escape the clone()d UTS namespace"


# DONE
if [ $EXIT_CODE -ne 0 ]
then
    echo "There appears to have been an error in the execution of this script: $EXIT_CODE"
fi
exit $EXIT_CODE
