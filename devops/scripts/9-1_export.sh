#!/bin/bash

# This script was made to help automate exporting SKID output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Prints identifying information
# 2. Runs the build system (which also executes the Check-based unit tests)
# 3. Executes the unit tests with Valgrind
# 4. Counts the number of unit tests
# 5. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
# 5.A. Listing sockets
# 5.B. Listing current processes
# 5.C. Viewing CPU information, usage, etc.
# 5.D. View information about the system
# 5.E. Obtain information about the logged on user
# 5.F. View the system's IP address(es)


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
    RET_CODE=0                                                # Exit code from command execution
    FORMAT_CHAR="${BOOKEND:0:1}"                               # Formatting
    TITLE="$@"                                                 # The title argument
    LENGTH=$((${#BOOKEND} + 1 + ${#TITLE} + 1 + ${#BOOKEND}))  # Length of the title
    BANNER=$(printf "%${LENGTH}s" | tr " " "$FORMAT_CHAR")     # Banner string

    # DO IT
    printf "\n%s\n" "$BANNER"  # Header
    print_title "$TITLE"
    RET_CODE=$?
    printf "%s\n" "$BANNER"  # Footer

    # DONE
    return $RET_CODE
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
    RET_CODE=0    # Exit code from command execution
    TITLE="$@"     # The title argument

    # DO IT
    printf "%s %s %s\n" "$BOOKEND" "$TITLE" "$BOOKEND"
    RET_CODE=$?

    # DONE
    return $RET_CODE
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
    RET_CODE=0  # Exit code from command execution

    # DO IT
    # Echo
    printf "The full command is '%s'\n" "$@"
    printf "Command output:\n"
    # Execute
    bash -c "$@"
    RET_CODE=$?
    # Vertical whitespace
    printf "\n\n"

    # DONE
    return $RET_CODE
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
    RET_CODE=0                # Exit code from command execution

    # DO IT
    # Invoke Usage
    bash -c "$BASE_CMD"
    echo
    # Call run_manual_test_command()
    run_manual_test_command "$FULL_CMD"
    RET_CODE=$?

    # DONE
    return $RET_CODE
}

BOOKEND="***"                                                       # Formatting
TEMP_RET=0                                                          # Temporary exit code var
EXIT_CODE=0                                                         # Exit value
DIST_DIR=./code/dist/                                               # Dist directory
SOCKET_SERVER="${DIST_DIR}test_sn_simple_stream_server.bin"         # Streaming socket server

# 1. Stores the date
printf "\n%s This output was created by %s on %s %s\n" "$BOOKEND" "$(basename "$0")" "$(date)" "$BOOKEND" && \
# 2. Runs the build system (which also executes the Check-based unit tests)
make && echo && \
# 3. Executes the unit tests with Valgrind
./devops/scripts/run_valgrind.sh; [[ $? -ne 0 ]] && exit; echo && \
# 4. Counts the number of unit tests (by running them again)
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' && echo
# 5. Misc.
# 5.A. Listing sockets
print_banner "LISTING SOCKETS"
print_title "1. Start a server which opens a socket"
run_manual_test_command "${SOCKET_SERVER} &"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
sleep 1  # Give it a second...
print_title "2. List all sockets"
run_manual_test_command "ss --tcp --udp --listening --numeric --processes"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "3. Shutdown the server"
run_manual_test_command "kill `pidof ${SOCKET_SERVER}`"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
# 5.B. Listing current processes
print_banner "LISTING CURRENT PROCESSES"
print_title "List every process on the system using BSD syntax"
run_manual_test_command "ps axu"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
# 5.C. Viewing CPU information, usage, etc.
print_banner "VIEWING CPU INFORMATION"
print_title "Display information about the CPU architecture"
run_manual_test_command "lscpu"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
# 5.D. View information about the system
print_banner "VIEW SYSTEM INFORMATION"
print_title "Print all system information"
run_manual_test_command "uname -a"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Query and change the system hostname and related settings"
run_manual_test_command "hostnamectl"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
# 5.E. Obtain information about the logged on user
print_banner "OBTAIN USER INFORMATION"
print_title "Show who is logged on and what they are doing"
run_manual_test_command "w"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Show who is logged on"
run_manual_test_command "who"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Print real and effective user and group IDs"
run_manual_test_command "id"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
# 5.F. View the system's IP address(es)
print_banner "VIEW SYSTEM IP ADDRESSES"
print_title "Show addresses assigned to all network interfaces"
run_manual_test_command "ip addr show"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# DONE
if [ $EXIT_CODE -ne 0 ]
then
    echo "There appears to have been an error in the execution of this script: $EXIT_CODE"
fi
exit $EXIT_CODE
