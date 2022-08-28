.DEFAULT_GOAL := help

# Bash color codes
COLOR_RED 	  := \033[0;31m
COLOR_GREEN   := \033[0;32m
COLOR_YELLOW  := \033[0;33m
COLOR_CYAN 	  := \033[0;36m
COLOR_NC 	  := \033[0m

# SoC target to compile for       (to be oerridden)
soctarget ?= None

# example to compile              (to be overridden)
ex ?= None

# Vuart for make run example      (may be overridden)
vuart ?= None

# Save objdump while compiling    (may be overridden)
objdump ?= false

########################################################################
# Checks

__check_env = true
__check_soctarget = true
__check_eg = true

ifeq ($(MAKECMDGOALS),)  # default target = help
    __check_env = false
    __check_soctarget = false
    __check_eg = false
endif

ifeq ($(MAKECMDGOALS),help)
    __check_env = false
    __check_soctarget = false
    __check_eg = false
endif

ifeq ($(MAKECMDGOALS),clean)
    __check_env = false
    __check_soctarget = false
endif

ifeq ($(MAKECMDGOALS),all)
    __check_soctarget = false
    __check_eg := false
endif

ifeq ($(MAKECMDGOALS),clean-all)
    __check_env = false
    __check_soctarget = false
    __check_eg = false
endif

ifeq ($(MAKECMDGOALS),run-all)
    __check_soctarget = false
    __check_eg = false
endif

#### check environment #####
ifeq ($(__check_env),true)
    # check if RVATOM env variable is set
    ifeq ($(RVATOM),)
        $(error RVATOM environment variable not set; did you forget to source the sourceme script?)
    endif
endif


#### check soctarget ####
ifeq ($(__check_soctarget),true)

    ifeq (${soctarget},None)  # if soctarget not overridden
        # check if atomsim exists
        ifeq ("$(wildcard $(RVATOM)/sim/build/bin/atomsim)", "")
            $(error soctarget not defined (failed to autodetect, need to build atomsim first))
        endif

        # autodetect soctarget from atomsim
        soctarget := $(shell atomsim --soctarget)
        $(info Autodetected soctarget from atomsim: $(soctarget))
    endif


    # check for validity of soctarget & select compile script
    ifeq ($(soctarget),atombones)
        COMPILE_SCRIPT = $(RVATOM)/scripts/compile-atombones
    else
        ifeq ($(soctarget),hydrogensoc)
            COMPILE_SCRIPT = $(RVATOM)/scripts/compile-hydrogensoc
        else
            $(error Invalid SoC target target $(soctarget))
        endif
    endif

endif


#### check ex ####
ifeq ($(__check_eg), true)

    ifeq ($(ex), None)
        $(error Please specify an example application)
    else
        ifeq ($(shell if [ -d $(ex) ]; then echo "true"; else echo "false"; fi), false)	# check if example exists
            $(error Example "$(ex)" does not exist)
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



########################################################################
# all examples directories
EXAMPLE_DIRS = $(sort $(dir $(wildcard */)))
SRC_FILES = $(patsubst %, $(ex)/%, $(src_files))

# atomsim args
ATOMSIM_ARGS = --maxitr=999999999

# VUART setting
ifneq ($(vuart), None)
    ATOMSIM_ARGS += --vuart=$(vuart)
endif

.DEFAULT_GOAL := help
# Query the default goal.
ifeq ($(.DEFAULT_GOAL),)
	$(warning no default goal is set)
endif

.PHONY: help
help:										## Show help
	@echo "*** Makefile for compiling & running examples ***"
	@echo "Usage:"
	@echo "	$$ make soctarget=<SOCTARGET> ex=<EXAMPLE> [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n", $$1, $$2}'
	@echo ""
	@echo "SOCTARGETs:"
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" atombones ""
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" hydrogensoc ""
	@echo ""
	@echo "EXAMPLEs (auto-detected):"
	@echo "	$(patsubst %/,%,${EXAMPLE_DIRS})"
	@echo ""
	@echo "Note:"
	@echo " - Following TARGETs can be run without overriding \"ex\" & \"soctarget\" variables"
	@echo "	help, all, clean-all"
	@echo " - To save objdump,, set the \"objdump\" variable to \"true\" in the CLI."
	@echo " - To save run using vuart, set the vuart variable to the path of the simport"
    



.PHONY: compile
compile: $(ex)/$(executable)				## Compile specified example

$(ex)/$(executable): $(SRC_FILES)
	@printf "$(COLOR_CYAN)%-15s | Compiling   %-35s <-  %s$(COLOR_NC)\n" ${ex} $@ $<
	cd $(ex) && $(COMPILE_SCRIPT) $(CFLAGS) $(src_files) -o $(executable) $(LFLAGS)

ifeq ($(objdump), true)
	riscv64-unknown-elf-objdump -htd $@ > $@.objdump
endif


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
	@$(patsubst %/, make -s ex=% compile;,${EXAMPLE_DIRS})


run-all:									## Build & run all examples
	@$(patsubst %/, echo "${COLOR_CYAN}----------------------------------------------------${COLOR_NC}"; make -s ex=% run;,${EXAMPLE_DIRS})


.PHONY: clean-all
clean-all:									## Clean all executables
	@$(patsubst %/, make -s ex=% clean;,${EXAMPLE_DIRS})
