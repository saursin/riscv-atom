# Atomsim target (to be oerridden):	Default target is auto-detected
soctarget ?= $(shell atomsim --soctarget)

# current example (to be overridden)
ex ?= None

# Vuart for make run example
vuart ?= None

# all examples directories
EXAMPLE_DIRS = $(sort $(dir $(wildcard */)))
SRC_FILES = $(patsubst %, $(ex)/%, $(src_files))

# atomsim args
ATOMSIM_ARGS = --maxitr=999999999


# Bash color codes
COLOR_RED 		= \033[0;31m
COLOR_GREEN 	= \033[0;32m
COLOR_YELLOW 	= \033[0;33m
COLOR_CYAN 	    = \033[0;36m
COLOR_NC 		= \033[0m


# Ignore following targets for requirement of ex
ifeq ($(MAKECMDGOALS), all)

else
ifeq ($(MAKECMDGOALS), help)

else
ifeq ($(MAKECMDGOALS), clean-all)

else
ifeq ($(MAKECMDGOALS), run-all)

else
ifeq ($(ex), None)
$(error Please specify an example application)
else
ifeq ($(shell if [ -d $(ex) ]; then echo "true"; else echo "false"; fi), false)	# check if example exists
$(error Example does not exist)
else
ifeq ($(shell if [ -f $(ex)/Makefile.include ]; then echo "true"; else echo "false"; fi), false)	# check if example contains Makefile.include
$(error Ignoring example due to missing Makefile.include)
else
# include coresponding makefile.include file
include $(ex)/Makefile.include						
endif
endif
endif
endif
endif
endif
endif


# select compile script
ifeq ($(soctarget), atombones)
COMPILE_SCRIPT = compile-atombones
else
ifeq ($(soctarget), hydrogensoc)
COMPILE_SCRIPT = compile-hydrogensoc
else
$(error Invalid SoC target target: $(soctarget))
endif
endif


# VUART setting
ifneq ($(vuart), None)
ATOMSIM_ARGS += --vuart=$(vuart)
endif


default: $(ex)/$(executable) # compile


.PHONY : help
help : Makefile								## show help
	@echo "*** Makefile for compiling & running examples ***"
	@echo "Usage:"
	@echo "	$$ make soctarget=<atombones/hydrogensoc> ex=<EXAMPLE> [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "EXAMPLEs (auto-detected):"
	@echo "	$(patsubst %/,%,${EXAMPLE_DIRS})"
	@echo ""
	@echo "Note:"
	@echo " - To compile an example don't specify TARGET"
	@echo " - Following TARGETs can be run without overriding \"ex\" & \"soctarget\" variables"
	@echo "	help, all, clean-all"

	
$(ex)/$(executable): $(SRC_FILES)
	@echo "${COLOR_CYAN}>> Building \"${ex}\" example"
	@printf "$(COLOR_CYAN)Compiling   %-35s <-  %s$(COLOR_NC)\n" $@ $<
	cd $(ex) && $(COMPILE_SCRIPT) $(CFLAGS) $(src_files) -o $(executable) $(LFLAGS)


run: $(ex)/$(executable)					## Run example
	@echo "${COLOR_CYAN}Running \"${ex}\" example ${COLOR_NC}"
	atomsim $(ATOMSIM_ARGS) $^


objdump: $(ex)/$(executable)				## Dump disassembly
	riscv64-unknown-elf-objdump -htd $^ > $^.objdump


.PHONY: clean
clean:										## Clean executable
	@echo "${COLOR_CYAN}Cleaning \"${ex}\" example binaries ${COLOR_NC}"
	rm -f $(ex)/$(executable)


all:										## Build all examples
	@$(patsubst %/, make ex=%;,${EXAMPLE_DIRS})


run-all:									## Build & run all examples
	@$(patsubst %/, echo "${COLOR_CYAN}----------------------------------------------------${COLOR_NC}"; make -s ex=% run;,${EXAMPLE_DIRS})


.PHONY: clean-all
clean-all:									## Clean all executables
	@$(patsubst %/, make -s ex=% clean;,${EXAMPLE_DIRS})
