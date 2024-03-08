##########################
###### INSTRUCTIONS ######
##########################
#
# 1. Do everything in $(CODE_DIR)
# 2. make
# 3. Watch for errors
# 4. Check $(CODE_DIR)$(DIST_DIR) for your binaries
# 5. Modifiy $(CODE_DIR)Makefile_linux with special rules
#
# Global "constants" are defined in code/Makefile_constants
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
SKIP_MF_ARGS = --directory=$(CODE_DIR)

.PHONY: all compile clean validate


##########################
##### MAKEFILE RULES #####
##########################
all:
	@clear
	$(CALL_MAKE) $(SKIP_MF_ARGS)

compile:
	@clear
	$(CALL_MAKE) $(SKIP_MF_ARGS) compile

clean:
	@clear
	$(CALL_MAKE) $(SKIP_MF_ARGS) clean

validate:
	@clear
	$(CALL_MAKE) $(SKIP_MF_ARGS) validate
