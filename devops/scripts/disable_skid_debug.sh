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
BAK_FILE_EXT=".bak"        # Backup file extension
UPDATE_COUNT=0             # Number of files updated
EXIT_CODE=0                # Exit code var

# 1. Verify the working tree is clean
if [[ `git status -s | wc -l` -ne 0 ]]
then
    echo -e "\nYour working tree does not appear to be clean.\nStage your commits and track your files before proceeding.\n"
    # EXIT_CODE=1
fi

# 2. Cleanup previously backed up files
if [[ $EXIT_CODE -eq 0 ]]
then
    for skid_backup_file in $(ls $SRC_CODE_DIR/*$BAK_FILE_EXT)
    do
        echo "Removing old backup file: $skid_backup_file"
        rm --force $skid_backup_file
        EXIT_CODE=$?
        if [[ $EXIT_CODE -ne 0 ]]
        then
            echo "The rm command encountered an error deleting $skid_backup_file"
            break
        fi
    done
fi

# 3. Update all the production code
if [[ $EXIT_CODE -eq 0 ]]
then
    for skid_source_file in $(ls $SRC_CODE_DIR/skid_*$SRC_FILE_EXT 2> /dev/null)
    do
        # Comments out the SKID_DEBUG macro, in place, after backing up the original source in-place
        sed -i"./*$BAK_FILE_EXT" 's/^#define SKID_DEBUG/\/\/ #define SKID_DEBUG/g' $skid_source_file
        EXIT_CODE=$?
        if [[ $EXIT_CODE -ne 0 ]]
        then
            echo -e "\nThe sed command has failed on $skid_source_file!\n"
            break
        else
            UPDATE_COUNT+=1
        fi
    done
fi

# 4. Provide feedback on changes
if [[ `git status -s | wc -l` -ne 0 && $UPDATE_COUNT -gt 0 ]]
then
    echo -e "\nThe following files were modified:"
    git status -s
    echo -e "\nTo undo these changes..."
    echo -e "\tgit restore $SRC_CODE_DIR/*$SRC_FILE_EXT"
    echo -e "\nTo commit these changes..."
    echo -e "\tgit add $SRC_CODE_DIR/*$SRC_FILE_EXT && git commit"
    echo -e "\nThe original files have been backed up..."
    echo -e "\tls $SRC_CODE_DIR/*$BAK_FILE_EXT"
fi

# DONE
exit $EXIT_CODE
