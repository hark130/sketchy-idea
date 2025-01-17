#!/bin/bash

# This script was made to help automate exporting 10-1's netstat output into a single
# text file to use as "proof" for my mentor.  It performs and stores various netstat commands.
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
TEMP_PID=""    # Temporary variable for dynamic PIDs

# Netstat Commands
# Show All Active Connections
printf "%s Show All Active Connections %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "netstat -a"

# List Listening Ports (and associated processes)
printf "%s List Listening Ports %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "netstat -lpn"
TEMP_PID=$(pidof test_sn_simple_dgram_server.bin)
if [[ $? -eq 0 ]]
then
    printf "%s Connect PID $TEMP_PID to its Test Binary %s\n" "$BOOKEND" "$BOOKEND"
    run_manual_test_command "cat /proc/$TEMP_PID/cmdline"
fi
TEMP_PID=$(pidof test_sn_simple_stream_server.bin)
if [[ $? -eq 0 ]]
then
    printf "%s Connect PID $TEMP_PID to its Test Binary %s\n" "$BOOKEND" "$BOOKEND"
    run_manual_test_command "cat /proc/$TEMP_PID/cmdline"
fi

# Display Routing Table
printf "%s Display Routing Table %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "netstat -r"

# Show Interface Statistics
printf "%s Show Interface Statistics %s\n" "$BOOKEND" "$BOOKEND"
run_manual_test_command "netstat -i"
