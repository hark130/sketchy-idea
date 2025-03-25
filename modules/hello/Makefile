##########################
###### INSTRUCTIONS ######
##########################
#
# 1. make -C modules/hello
# 2. Watch for errors
# 3. Check ????? for your kernel modules
#


##########################
### MAKEFILE VARIABLES ###
##########################

### OS-DEPENDENT INCLUSION ###
ifeq ($(OS),Windows_NT)
$(error Wrong operating system.  This is $(OS).)
else
# include $(abspath ../Makefile_constants)
# include $(PWD)/../Makefile_constants
SHELL = /bin/bash
# Ubuntu
KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)
#KERNEL_DIR = /usr/src/linux-headers-5.4.0-187
# Fedora
# KERNEL_DIR = /usr/src/kernels/$(shell uname -r)
endif

### FILE EXTENSIONS ###
HDR_FILE_EXT = .h
LIB_FILE_EXT = .lib
OBJ_FILE_EXT = .o
SRC_FILE_EXT = .c

### DIRECTORIES ###
# Relative path to the directory holding the kernel module directories
CODE_DIR = modules/

### MAKEFILE ARGUMENTS ###
MF_ARGS = --no-print-directory
CALL_MAKE = $(MAKE) $(MF_ARGS)

###############################
### KERNEL MODULE VARIABLES ###
###############################

EXTRA_CFLAGS = -I/usr/include/$(uname -m)-linux-gnu
UNAME_M := $(shell uname -m)
MODULE_NAME = hello
obj-m += $(MODULE_NAME)$(OBJ_FILE_EXT)
CWD = $(PWD)/$(CODE_DIR)$(MODULE_NAME)/

.PHONY: all build clean reset teardown test


##########################
##### MAKEFILE RULES #####
##########################
# Do everything from scratch
all:
	$(CALL_MAKE) reset
	$(CALL_MAKE) build
	$(CALL_MAKE) test

# Build the kernel module
build:
	$(CALL_MAKE) -I/usr/include/$(UNAME_M)-linux-gnu -C $(KERNEL_DIR) M=$(PWD) modules

# Remove non-source controlled files
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *-

# Reset all files back to the starting state
reset:
	$(CALL_MAKE) teardown
	$(CALL_MAKE) clean

# Unload the kernel module
teardown:
	-sudo rmmod $(MODULE_NAME)

# Test the kernel module
test:
	$(CALL_MAKE) teardown

	# Clear the kernel log without echo
	sudo dmesg -C

	# Insert the module
	sudo insmod $(MODULE_NAME).ko

	# Display the kernel log
	dmesg
