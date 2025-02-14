#!/bin/bash

# This script was made to automate the process of globally(?) disabling SKID's DEBUG mode
# in the production code.
#
# EXIT CODE
#   0 on success
#   Non-zero value on error


TEMP_RET=0                                                     # Temporary exit code var

# TO DO: DON'T DO NOW... check for untracked files

# Update all the production code
for skid_source_file in $(ls code/src/skid_*.c)
do
    # Comments out the SKID_DEBUG macro, in place, after backing up the original source in-place
    sed -i'./*.bak' 's/^#define SKID_DEBUG/\/\/ #define SKID_DEBUG/g' $skid_source_file
    TEMP_RET=$?
    if [[ $TEMP_RET -ne 0 ]]
    then
        echo -e "\nThe sed command has failed on $skid_source_file!\n"
        break
    fi
done

# DONE
# TO DO: DON'T DO NOW... INCLUDE COPY/PASTE UNDO AND DO_IT() COMMANDS
exit $TEMP_RET
