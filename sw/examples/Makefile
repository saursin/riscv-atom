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

# Compile for simulation          (may be overridden)
sim ?= false

# Compile for use with bootloader (may be overridden)
boot ?= false

# Vuart for make run example      (may be overridden)
vuart ?= None

# Save objdump while compiling    (may be overridden)
objdump ?= true

# Save binary while compiling     (may be overridden)
bin ?= true

# Save map file while compiling   (may be overridden)
map ?= true

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


RVPREFIX := riscv64-unknown-elf
CFLAGS := -Wall
CFLAGS += -mabi=ilp32 -march=rv32i -nostartfiles -ffreestanding -Os
CFLAGS += -I $(RVATOM_LIB)/include -L $(RVATOM_LIB)

LINKERSCRIPT := None
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
        CFLAGS += -DTARGET_ATOMBONES
        LINKERSCRIPT:= $(RVATOM_LIB)/link/link.ld
    else
    ifeq ($(soctarget),hydrogensoc)
        CFLAGS += -DTARGET_HYDROGENSOC
        LINKERSCRIPT:= $(RVATOM_LIB)/link/link_hydrogensoc.ld
    else
        $(error Invalid SoC target target $(soctarget))
    endif
    endif

    ifeq ($(boot), true)
        LINKERSCRIPT:= ../flashboot/link_app.lds
    endif

endif

LFLAGS := -T $(LINKERSCRIPT) -lcatom -Wl,--gc-sections

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
ATOMSIM_ARGS = --maxitr=999999999 --enable-uart-dump

# VUART setting
ifneq ($(vuart), None)
    ATOMSIM_ARGS += --vuart=$(vuart)
endif

ifeq ($(map), true)
    LFLAGS += -Xlinker -Map $(executable).map
endif

ifeq ($(sim), true)
    CFLAGS += -DSIM
endif

.DEFAULT_GOAL := help
# Query the default goal.
ifeq ($(.DEFAULT_GOAL),)
	$(warning no default goal is set)
endif

.PHONY: help
help:										## Show help
	@printf "*** Makefile for compiling & running examples ***\n"
	@printf "Usage:"
	@printf "	$$ make soctarget=<SOCTARGET> ex=<EXAMPLE> [TARGET]\n"
	@printf "\n"
	@printf "TARGETs:\n"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n", $$1, $$2}'
	@printf "\n"
	@printf "SOCTARGETs:\n"
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" atombones ""
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" hydrogensoc ""
	@printf "\n"
	@printf "EXAMPLEs (auto-detected):\n"
	@printf "	$(patsubst %/,%,${EXAMPLE_DIRS})\n"
	@printf "\n"
	@printf "Note:\n"
	@printf " - Following TARGETs can be run without overriding \"ex\" & \"soctarget\" variables\n"
	@printf "	help, all, clean-all\n"
	@printf " - To save objdump,, set the \"objdump\" variable to \"true\" in the CLI.\n"
	@printf " - To save run using vuart, set the vuart variable to the path of the simport\n"
    



.PHONY: compile
compile: $(ex)/$(executable)				## Compile specified example

$(ex)/$(executable): $(SRC_FILES)
	@printf "$(COLOR_CYAN)%-15s | Compiling   %-35s <-  %s$(COLOR_NC)\n" ${ex} $@ $<
	cd $(ex) && $(RVPREFIX)-gcc $(CFLAGS) $(src_files) -o $(executable) $(LFLAGS)

ifeq ($(objdump), true)
	$(RVPREFIX)-objdump -htd $@ > $@.objdump
endif

ifeq ($(bin), true)
	$(RVPREFIX)-objcopy -O binary $@ $@.bin
	@printf "Binary size: %s bytes\n" $$(stat --format=%s "$@.bin")
endif


run: $(ex)/$(executable)					## Run example
	@printf "${COLOR_CYAN}Running \"${ex}\" example ${COLOR_NC}\n"
	atomsim $(ATOMSIM_ARGS) $^


objdump: $(ex)/$(executable)				## Dump disassembly
	riscv64-unknown-elf-objdump -htd $^ > $^.objdump


.PHONY: clean
clean:										## Clean executable
	@printf "${COLOR_CYAN}Cleaning \"${ex}\" example binaries ${COLOR_NC}\n"
	rm -f $(ex)/$(executable) $(ex)/*.o $(ex)/*.map $(ex)/*.objdump


all:										## Build all examples
	@$(patsubst %/, make -s ex=% compile;,${EXAMPLE_DIRS})


run-all:									## Build & run all examples
	@$(patsubst %/, printf "${COLOR_CYAN}----------------------------------------------------${COLOR_NC}\n"; make -s ex=% run;,${EXAMPLE_DIRS})


.PHONY: clean-all
clean-all:									## Clean all executables
	@$(patsubst %/, make -s ex=% clean;,${EXAMPLE_DIRS})
