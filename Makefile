##########################
###### INSTRUCTIONS ######
##########################
#
# 1. Do everything in $(CODE_DIR)
# 2. `make`
# 3. Watch for errors
# 4. Check $(CODE_DIR)$(DIST_DIR) for your binaries
# 5. `make install` to install the SKETCHY IDEA (SKID) shared object
#
# NOTES:
# - Modifiy $(CODE_DIR)Makefile_linux with special rules
# - Global "constants" are defined in code/Makefile_constants
#


##########################
### MAKEFILE VARIABLES ###
##########################

### DIRECTORIES ###
# Relative path to the directory holding the code and its Makefile
CODE_DIR = ./code/

# This was made to avoid circular dependencies and redundancies
include $(CODE_DIR)Makefile_constants

### MAKEFILE ARGUMENTS ###
SKID_MF_ARGS = --directory=$(CODE_DIR)

.PHONY: all clean compile install release test validate


##########################
##### MAKEFILE RULES #####
##########################
all:
	$(CALL_MAKE) $(SKID_MF_ARGS)

clean:
	$(CALL_MAKE) $(SKID_MF_ARGS) clean

compile:
	$(CALL_MAKE) $(SKID_MF_ARGS) compile

install:
	$(CALL_MAKE) $(SKID_MF_ARGS) install

release:
	$(CALL_MAKE) $(SKID_MF_ARGS) release

test:
	$(CALL_MAKE) $(SKID_MF_ARGS) test

validate:
	$(CALL_MAKE) $(SKID_MF_ARGS) validate
