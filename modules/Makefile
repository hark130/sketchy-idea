##########################
###### INSTRUCTIONS ######
##########################
#
# 1. Make a directory per-kernel module
# 2. Write your code in that directory
# 3. Write a Makefile defining, at a minimum, the following rules:
#	- all       # Do everything
#	- build     # Build the kernel module
#	- clean     # Remove non-source controlled files
#	- reset     # Reset all files back to the starting state
#	- teardown  # Unload the kernel module
#	- test      # Automate kernel module testing
# 4. make lkm (or make -C modules/)
# 5. Watch for errors
# 6. Check ????? for your kernel modules
#


##########################
### MAKEFILE VARIABLES ###
##########################

### OS-DEPENDENT INCLUSION ###
ifeq ($(OS),Windows_NT)
$(error Wrong operating system.  This is $(OS).)
else
include Makefile_constants
endif

### DIRECTORIES ###
# List all kernel module directories
MODULE_LIST := $(shell ls -d */)

.PHONY: all build clean reset teardown test


##########################
##### MAKEFILE RULES #####
##########################
all:
	for module in $(MODULE_LIST); do \
		$(CALL_MAKE) -C $$module all; \
	done

build:
	for module in $(MODULE_LIST); do \
		$(CALL_MAKE) -C $$module build; \
	done

clean:
	for module in $(MODULE_LIST); do \
		$(CALL_MAKE) -C $$module clean; \
	done

reset:
	for module in $(MODULE_LIST); do \
		$(CALL_MAKE) -C $$module reset; \
	done

teardown:
	for module in $(MODULE_LIST); do \
		$(CALL_MAKE) -C $$module teardown; \
	done

test:
	for module in $(MODULE_LIST); do \
		$(CALL_MAKE) -C $$module test; \
	done
