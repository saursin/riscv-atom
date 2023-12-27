#h# ***** RISC-V Atom Examples Makefile *****

#v# Specify soctarget (autodetected if not provided)
soctarget?= None

#v# Specify example to build (compulsory)
ex?= None

#v# Build for simulation
sim?= 1

#v# Vuart for make run example
vuart?= None

#v# Save map file while compiling
map?= 1

########################################################################
# Checks

__check_soctarget = true
__check_eg = true

ifeq ($(MAKECMDGOALS),)  # default target = help
    __check_soctarget = false
    __check_eg = false
endif

ifeq ($(MAKECMDGOALS),help)
    __check_soctarget = false
    __check_eg = false
endif

ifeq ($(MAKECMDGOALS),clean)
    __check_soctarget = false
endif

ifeq ($(MAKECMDGOALS),all)
    __check_soctarget = false
    __check_eg := false
endif

ifeq ($(MAKECMDGOALS),clean-all)
    __check_soctarget = false
    __check_eg = false
endif

ifeq ($(MAKECMDGOALS),run-all)
    __check_soctarget = false
    __check_eg = false
endif

_mk_check_env = 1
include ../../common.mk
########################################################################
RVPREFIX := riscv64-unknown-elf
CFLAGS += -Wall
LINKERSCRIPT := None

#### check soctarget ####
ifeq ($(__check_soctarget), true)

    ifeq (${soctarget},None)  # if soctarget not overridden
        # check if atomsim exists
        ifeq ("$(wildcard $(RVATOM)/sim/build/bin/atomsim)", "")
            $(error soctarget not defined (failed to autodetect, need to build atomsim first))
        endif

        # autodetect soctarget from atomsim
        soctarget := $(shell atomsim --soctarget)
        $(info Autodetected soctarget from atomsim: $(soctarget))
    endif

    CFLAGS+= -DTARGET_$(shell echo $(soctarget) | tr 'a-z' 'A-Z')
    LINKERSCRIPT:= $(RVATOM_LIB)/link/link_$(soctarget).ld
    ISA:=$(shell $(RVATOM)/scripts/cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -a isa)
    ABI:=$(shell $(RVATOM)/scripts/cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -a abi)
endif

CFLAGS += -march=$(ISA) -mabi=$(ABI) -nostartfiles -ffreestanding -Os
CFLAGS += -I $(RVATOM_LIB)/include -L $(RVATOM_LIB)
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
ATOMSIM_ARGS = --maxitr=999999999 -uv

# VUART setting
ifneq ($(vuart), None)
    ATOMSIM_ARGS += --vuart=$(vuart)
endif

ifeq ($(map), 1)
    LFLAGS += -Xlinker -Map $(executable).map
endif

ifeq ($(sim), 1)
    CFLAGS += -DSIM
endif


.PHONY: compile
compile: $(ex)/$(executable)    			#t# Compile specified example

$(ex)/$(executable): $(SRC_FILES)
	$(call print_msgt,Building)
	cd $(ex) && $(RVPREFIX)-gcc $(CFLAGS) $(src_files) -o $(executable) $(LFLAGS)
	$(RVPREFIX)-objdump -Shtd $@ > $(basename $@).lst
	$(RVPREFIX)-objcopy -O binary $@ $(basename $@).bin
	@printf "$(CLR_BL)$(CLR_B)  Binary size:$(CLR_NB) %s bytes$(CLR_NC)\n" $$(stat --format=%s "$(basename $@).bin")

.PHONY: run
run: $(ex)/$(executable)					#t# Run example
	$(call print_msg,Running, $<)
	atomsim $(ATOMSIM_ARGS) $^


.PHONY: dism
dism: $(ex)/$(executable)				    #t# Dump disassembly
	$(call print_msg,Generating disassembly,$(basename $^).lst)
	riscv64-unknown-elf-objdump -htd $^ > $(basename $^).lst


# .PHONY: prog
# prog: $(ex)/$(executable).bin               #t# Program fpga flash
# 	prog_firmware.sh $^


.PHONY: clean
clean:										#t# Clean executable
	$(call print_msg,Cleaning,$(ex))
	rm -f $(ex)/$(executable) $(ex)/*.o $(ex)/*.map $(ex)/*.lst


all:										#t# Build all examples
	@$(patsubst %/, make -s ex=% sim=1 compile;,${EXAMPLE_DIRS})


run-all:									#t# Build & run all examples
	@$(patsubst %/, printf "${CLR_CY}----------------------------------------------------${CLR_NC}\n"; make -s ex=% sim=1 run;,${EXAMPLE_DIRS})


.PHONY: clean-all
clean-all:									#t# Clean all executables
	@$(patsubst %/, make -s ex=% clean;,${EXAMPLE_DIRS})
