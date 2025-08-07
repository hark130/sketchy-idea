#!/bin/bash

# This script was made to help automate exporting SKID output into a single
# text file to use as "proof" for my mentor.  It performs the following:
#
# 1. Prints identifying information
# 2. Misc. (e.g., executing bespoke manual test code highlighting features/functionality)
# 2.A. Manage character and block devices found in /dev
# 2.B. Create a disk partition
# 2.C. Select and create a filesystem on a disk
# 2.D. Mount and unmount a disk
# 2.E. Mount a disk image
# 2.F. Create, mount, and unmount an encrypted disk


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
    RET_CODE=0                                                 # Exit code from command execution
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
#   Print script usage
# ARGUMENTS:
#   script name: Pass in $0
# RETURN:
#   The exit code
#
print_usage()
{
    # LOCAL VARIABLES
    RET_CODE=0   # Exit code from command execution
    SCRIPT="$@"  # The title argument

    # DO IT
    echo
    printf "%s\n" \
           "1. Plug in a disposable block device (e.g., thumb drive)" \
           "2. Determine the name of the device: lsblk" \
           "3. Unmount the device (if it's mounted): umount <MOUNT_POINT>" \
           "4. Delete any partitions: parted <BLOCK_DEVICE> rm 1" \
           "5. Call this script with the <BLOCK_DEVICE> name" \
           "" \
           "USAGE: $SCRIPT <BLOCK_DEVICE>"
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
REG_FILE=./code/test/test_input/regular_file.txt                    # Regular file for test input
NEEDLE=`head -n 1 ${REG_FILE}`                                      # A needle to search for as proof
TARGET_DEVICE=$1                                                    # Command line argument
CHAR_DEVICE=/dev/11-2_char_device                                   # New character device
BLOCK_DEVICE=/dev/11-2_block_device                                 # New block device
TARGET_LABEL=11-2_label
TARGET_MOUNT=/mnt/11-2_target                                       # Mount point for TARGET_LABEL
DISK_IMAGE=/tmp/11-2_disk.img                                       # Disk image file
DISK_MOUNT=/mnt/11-2_disk_img                                       # Mount point for DISK_IMAGE
PASSPHRASE=SemperInspectus                                          # Passphrase for the encrypted image
PASS_FILE=/tmp/passphrase                                           # Keyfile for cryptsetup
ENCR_IMAGE=/tmp/11-2_encr.img                                       # Encrypted disk image
ENCR_MAP_NAME=11-2_enc_disk                                         # Encrypted mapping base name
ENCR_MAPPING="/dev/mapper/${ENCR_MAP_NAME}"                         # Absolute mapping filename
ENCR_MNT=/mnt/11-2_enc_img                                          # Mounted encrypted mapping

# lsblk command for maximum detail
LSBLK_CMD="lsblk -f -o NAME,FSTYPE,LABEL,UUID,PARTTYPE,PARTLABEL,PARTUUID,RO,RM,TYPE,SIZE,MOUNTPOINT,MODEL,VENDOR,PHY-SeC,LOG-SEC,ROTA,TRAN,SERIAL"

# 0. Input Validation
if [[ -z "$TARGET_DEVICE" ]]
then
    echo "You neglected to provide a target device on the command line."
    EXIT_CODE=1
elif [[ ! -e "$TARGET_DEVICE" ]]
then
    echo "Unable to locate: $TARGET_DEVICE"
    EXIT_CODE=1
elif [[ ! -b "$TARGET_DEVICE" ]]
then
    echo "$TARGET_DEVICE is not a block device"
    EXIT_CODE=1
fi
# Validate
if [[ $EXIT_CODE -ne 0 ]]
then
    echo "You provided invalid input.  Exiting with $EXIT_CODE"
    print_usage $0
    exit $EXIT_CODE
else
    print_banner "WARNING"
    print_title "This device will be used to demonstrate device management"
    echo -e "All data on $TARGET_DEVICE will be destroyed.\n"
    read -p "Enter 'Y' to continue:  " acknowledge
    if [[ "$acknowledge" != "Y" ]]
    then
        echo "Exiting."
        exit $EXIT_CODE
    fi
fi


# 1. Stores the date
printf "\n%s This output was created by %s on %s %s\n" "$BOOKEND" "$(basename "$0")" "$(date)" "$BOOKEND"
# 2. Misc.
echo

# 2.A. Manage character and block devices found in /dev
print_banner "MANAGE CHARACTER DEVICES"
print_title "List character devices in /dev"
run_manual_test_command "ls -l /dev | grep '^c'"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a character device in /dev"
run_manual_test_command "mknod ${CHAR_DEVICE} c 318 90"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Let's see it"
run_manual_test_command "ls -l ${CHAR_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Let's get rid of it"
run_manual_test_command "unlink ${CHAR_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_banner "MANAGE BLOCK DEVICES"
print_title "List block devices in /dev"
run_manual_test_command "ls -l /dev | grep '^b'"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a block device in /dev"
run_manual_test_command "mknod ${BLOCK_DEVICE} b 16 0xACC"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Let's see it"
run_manual_test_command "ls -l ${BLOCK_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Let's get rid of it"
run_manual_test_command "unlink ${BLOCK_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# 2.B. Create a disk partition
print_banner "CREATE A DISK PARTITION"
print_title "BEFORE: Let's view the block device as-is"
run_manual_test_command "${LSBLK_CMD} ${TARGET_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a GPT partition table"
run_manual_test_command "parted ${TARGET_DEVICE} --script mklabel gpt"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a new primary partition"
run_manual_test_command "parted ${TARGET_DEVICE} --script mkpart primary 0% 100%"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# 2.C. Select and create a filesystem on a disk
print_banner "CREATE A FILESYSTEM ON DISK"
print_title "Build a new filesystem"
run_manual_test_command "mkfs.ext4 -F -L ${TARGET_LABEL} ${TARGET_DEVICE}1"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# 2.D. Mount and unmount a disk
print_banner "MOUNT THE PARTITION"
print_title "Create a mount point"
run_manual_test_command "mkdir -p ${TARGET_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Mount the block device partition to the mount point1"
run_manual_test_command "mount ${TARGET_DEVICE}1 ${TARGET_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Change the owner and group of the mount point"  # DEBUGGING
run_manual_test_command "chown ${SUDO_UID}:${SUDO_GID} ${TARGET_MOUNT}"  # DEBUGGING
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Verify it mounted"
run_manual_test_command "df -h | head -n 1 && df -h | grep ${TARGET_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Write a file to the mount"
run_manual_test_command "cp ${REG_FILE} ${TARGET_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "DURING: Let's view the block device now that it is mounted"
run_manual_test_command "${LSBLK_CMD} ${TARGET_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# echo "Press any key to continue..."  # DEBUGGING
# read -n 1 -s  # DEBUGGING
# echo "Script resumed!"  # DEBUGGING

print_banner "UNMOUNT THE PARTITION"
print_title "Unmount the filesystem"
run_manual_test_command "umount ${TARGET_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "AFTER: Let's view the block device now that it is unmounted"
run_manual_test_command "${LSBLK_CMD} ${TARGET_DEVICE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_banner "CLEANUP"
print_title "Remove the mount point"
run_manual_test_command "rmdir ${TARGET_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# 2.E. Mount a disk image
print_banner "DISK IMAGE"
print_title "Create a 100MB empty disk image"
run_manual_test_command "dd if=/dev/zero of=${DISK_IMAGE} bs=1M count=100"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a filesystem on the disk image"
run_manual_test_command "mkfs.ext4 ${DISK_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a mount point"
run_manual_test_command "mkdir -p ${DISK_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Mount the disk image as a loop device"
run_manual_test_command "mount -o loop ${DISK_IMAGE} ${DISK_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "View the new loop device"
run_manual_test_command "lsblk | head -n1 && lsblk | grep ${DISK_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Write a file to the mount"
run_manual_test_command "cp ${REG_FILE} ${DISK_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# echo "Press any key to continue..."  # DEBUGGING
# read -n 1 -s  # DEBUGGING
# echo "Script resumed!"  # DEBUGGING

print_banner "UNMOUNT THE PARTITION"
print_title "Unmount the filesystem"
run_manual_test_command "umount ${DISK_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_banner "PEEK INTO CLEARTEXT DISK IMAGE"
print_title "Find the file contents copied into the mounted image"
run_manual_test_command "strings ${DISK_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "More proof?  Let's grep for a string we know is in there."
run_manual_test_command "grep --ignore-case \"${NEEDLE}\" ${DISK_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_banner "CLEANUP"
print_title "Remove the mount point"
run_manual_test_command "rmdir ${DISK_MOUNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Delete the disk image"
run_manual_test_command "rm --force ${DISK_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# 2.F. Create, mount, and unmount an encrypted disk
print_banner "ENCRYPTED DISK IMAGE"
print_title "Create a 100MB empty disk image"
run_manual_test_command "dd if=/dev/zero of=${ENCR_IMAGE} bs=1M count=100"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a keyfile"
run_manual_test_command "echo -n ${PASSPHRASE} > ${PASS_FILE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Encyrpt the disk image with LUKS"
run_manual_test_command "cryptsetup luksFormat ${ENCR_IMAGE} --batch-mode --key-file=${PASS_FILE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a mapping with name ${ENCR_MAP_NAME} backed by device ${ENCR_IMAGE}"
run_manual_test_command "cryptsetup open ${ENCR_IMAGE} ${ENCR_MAP_NAME} --key-file=${PASS_FILE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Make a filesystem on the encrypted device"
run_manual_test_command "mkfs.ext4 ${ENCR_MAPPING}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Create a mount point"
run_manual_test_command "mkdir -p ${ENCR_MNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Mount the encrypted mapping as a loop device"
run_manual_test_command "mount -o loop ${ENCR_MAPPING} ${ENCR_MNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi

# echo "Press any key to continue..."  # DEBUGGING
# read -n 1 -s  # DEBUGGING
# echo "Script resumed!"  # DEBUGGING

print_title "Write a file to the mount"
run_manual_test_command "cp ${REG_FILE} ${ENCR_MNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Unmount the filesystem"
run_manual_test_command "umount ${ENCR_MNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Close the encrypted device"
run_manual_test_command "cryptsetup close ${ENCR_MAP_NAME}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "The disk image is encrypted"
run_manual_test_command "cryptsetup luksDump ${ENCR_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Need more proof?  Let's pull the strings."
run_manual_test_command "strings ${ENCR_IMAGE} | head"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
echo -e "[OUTPUT TRUNCATED]\n"
print_title "MORE proof?  Let's grep for a string we know is in there."
run_manual_test_command "grep --invert-match --ignore-case \"${NEEDLE}\" ${ENCR_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_banner "CLEANUP"
print_title "Remove the mount point"
run_manual_test_command "rmdir ${ENCR_MNT}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi
print_title "Delete the disk image"
run_manual_test_command "rm --force ${ENCR_IMAGE}"
TEMP_RET=$?; if [[ $TEMP_RET -ne 0 ]]; then EXIT_CODE=$TEMP_RET; echo -e "Command failed!\n"; fi


# DONE
if [ $EXIT_CODE -ne 0 ]
then
    echo "There appears to have been an error in the execution of this script: $EXIT_CODE"
fi
exit $EXIT_CODE
