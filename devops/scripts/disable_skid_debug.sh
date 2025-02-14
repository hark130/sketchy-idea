#!/bin/bash

# This script was made to automate the process of globally(?) disabling SKID's DEBUG mode
# in the production code.
#
# EXIT CODE
#   0 on success
#   1 if the working tree is not clean (see: git status -s)
#   Non-zero value on error


SRC_CODE_DIR="./code/src"  # Source code directory
SRC_FILE_EXT=".c"          # Source code file extension
EXIT_CODE=0                # Exit code var

# 1. Verify the working tree is clean
if [[ `git status -s | wc -l` -ne 0 ]]
then
    echo -e "\nYour working tree does not appear to be clean.\nStage your commits and track your files before proceeding.\n"
    EXIT_CODE=1
fi

# 2. Update all the production code
if [[ $EXIT_CODE -eq 0 ]]
then
    for skid_source_file in $(ls $SRC_CODE_DIR/skid_*$SRC_FILE_EXT)
    do
        # Comments out the SKID_DEBUG macro, in place, after backing up the original source in-place
        sed -i'./*.bak' 's/^#define SKID_DEBUG/\/\/ #define SKID_DEBUG/g' $skid_source_file
        EXIT_CODE=$?
        if [[ $EXIT_CODE -ne 0 ]]
        then
            echo -e "\nThe sed command has failed on $skid_source_file!\n"
            break
        fi
    done
fi

# 3. Check for changes
if [[ `git status -s | wc -l` -ne 0 ]]
then
    echo -e "\nThe following files were modified:"
    git status -s
    echo -e "\nTo undo these changes..."
    echo -e "\tgit restore $SRC_CODE_DIR/*$SRC_FILE_EXT"
    echo -e "\nTo commit these changes..."
    echo -e "\tgit add $SRC_CODE_DIR/*$SRC_FILE_EXT && git commit"
fi

# DONE
exit $EXIT_CODE
