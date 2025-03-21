##########################
###### INSTRUCTIONS ######
##########################
#
# 1. make -C modules/query
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
include $(abspath ../Makefile_constants)
# Ubuntu
KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)
# Fedora
# KERNEL_DIR = /usr/src/kernels/$(shell uname -r)
endif

###############################
### KERNEL MODULE VARIABLES ###
###############################

MODULE_NAME = query
obj-m += $(MODULE_NAME)$(OBJ_FILE_EXT)

.PHONY: all build clean reset teardown test


##########################
##### MAKEFILE RULES #####
##########################
# Do everything from scratch
all:
	@echo "All"
	$(CALL_MAKE) reset
	$(CALL_MAKE) build
	$(CALL_MAKE) test

# Build the kernel module
build:
	@echo "build"
	$(CALL_MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

# Remove non-source controlled files
clean:
	@echo "clean"
	rm -rf *.o *.ko *.mod.* *.symvers *.order *-

# Reset all files back to the starting state
reset:
	@echo "reset"
	$(CALL_MAKE) teardown
	$(CALL_MAKE) clean

# Unload the kernel module
teardown:
	@echo "teardown"
	# We put a — in front of the rmmod command to tell make to ignore
	# an error in case the module isn’t loaded.
	-sudo rmmod $(MODULE_NAME)

# Test the kernel module
test:
	@echo "test"
	$(CALL_MAKE) teardown

	# Clear the kernel log without echo
	sudo dmesg -C

	# Insert the module
	sudo insmod $(MODULE_NAME).ko

	# Display the kernel log
	dmesg
