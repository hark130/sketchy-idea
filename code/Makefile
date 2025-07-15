##########################
###### INSTRUCTIONS ######
##########################
#
# 1. Write your code
# 2. make (or make -C code/)
# 3. Watch for errors
# 4. Check code/$(DIST_DIR) for your binaries
# 5. Modifiy Makefile_linux with special rules
#
# Global "constants" are defined in Makefile_constants.mk
#


##########################
### MAKEFILE VARIABLES ###
##########################

### OS-DEPENDENT INCLUSION ###
ifeq ($(OS),Windows_NT)
$(error Wrong operating system.  This is $(OS).)
else
include Makefile_linux.mk
endif

.PHONY: all compile clean test validate


##########################
##### MAKEFILE RULES #####
##########################
all:
	$(CALL_MAKE) validate
	$(CALL_MAKE) clean
	$(CALL_MAKE) compile
	$(CALL_MAKE) test
	$(CALL_MAKE) release

clean:
	@echo ""
	@echo "CLEANING"
	$(CALL_MAKE) _clean

compile:
	@echo ""
	@echo "COMPILING"
	$(CALL_MAKE) _compile

install:
	@echo ""
	@echo "INSTALLING"
	$(CALL_MAKE) _install

release:
	@echo ""
	@echo "RELEASING"
	$(CALL_MAKE) _release

test:
	@echo ""
	@echo "RUNNING TEST CASES"
	$(CALL_MAKE) _test

validate:
	@echo ""
	@echo "VALIDATING"
	$(CALL_MAKE) _validate