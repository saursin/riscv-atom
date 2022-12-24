#====================================================================#
#        ____  _________ _______    __         __                    #
#       / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___     #
#      / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \    #
#     / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /    #
#    /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/     #
#                                                                    #
#====================================================================#
# README:
#	This is the root makefile for the RISC-V Atom project. It 
#	provides the convenience of building sim, tools, lib, etc from 
#	the root of the project directory. see `$ make help` for info 
#	on how to usage instructions and available targets
#
#====================================================================#
# About this makefile:
#	This is a self-documenting Makefile. It parses the target 
# 	descriptions from the makefile autoimetically to display when 
#	`$ make help` is called. To enable this, One-line target 
#	description should be prefixed with double hash(#), and should 
#	placed be in same line as target recepie. lines prefixed with 
#	#h and #f will be placed at the header abnd footer of help page
#	respectively.
#====================================================================#
include common.mk

# soctarget variable (should be overridden using CLI)
soctarget ?= atombones

# Directories
sim_dir  	:= sim
scar_dir 	:= test/scar
elfdump_dir := tools/elfdump
lib_dir		:= sw/lib
doxy_dir	:= sim/docs

# Flags to the makefiles
MKFLAGS := -s

# Check if RVATOM env variable is set
ifeq ($(RVATOM),)
    $(error RVATOM environment variable not set; did you forget to source the sourceme script?)
endif

#======================================================================
# Recepies
#======================================================================

default: sim lib   					## Build sim, elfdump, and libs
	@printf "\n$(CLR_GR)==============================\n"
	@printf "       Build Succesful!\n"
	@printf "==============================$(CLR_NC)\n"
	@printf " - atomsim [$(soctarget)]\n"
	@printf " - software libraries\n"

all : doxy-pdf default				## Build default with docs
	@printf " - doxygen-docs in latex, html & pdf\n"


#======== Help ========
.PHONY : help
help : Makefile						## Show help message
	@printf "****** RISC-V Atom Makefile ******\n"
	@printf "Usage:\n"
	@printf "	$$ make soctarget=[SOCTARGET] [TARGET]\n"
	@printf "\n"
	
	@printf "SOCTARGETs:\n"
	@printf "\t$(CLR_CY)%-20s$(CLR_NC) %s\n" "atombones" "A barebone Atom based SoC with simulated memories\n"
	@printf "\t$(CLR_CY)%-20s$(CLR_NC) %s\n" "hydrogensoc" "A minimal Atom based SoC\n"

	@printf "\n"
	@printf "TARGETs:\n"
	@grep -E -h '\s##\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\t$(CLR_CY)%-20s$(CLR_NC) %s\n", $$1, $$2}'
	@printf "\n"
	@sed -n 's/^#f //p' Makefile


# ======== AtomSim ========
.PHONY : sim
sim:                       			## Build atomsim for given target [default: atombones]
	@printf "$(CLR_GR)>> Building Atomsim [soctarget:$(soctarget)] $(CLR_NC)\n"
	make $(MKFLAGS) -C $(sim_dir) soctarget=$(soctarget) DEBUG=1


.PHONY: clean-sim
clean-sim:							## Clean atomsim build files
	@printf "$(CLR_GR)>> Cleaning atomsim build files $(CLR_NC)\n"
	make $(MKFLAGS) -C $(sim_dir)  soctarget=$(soctarget) clean


# ======== SCAR ========
.PHONY: scar     			
scar: $(bin_dir)/$(sim_executable)	## Verify target using scar
	@printf "$(CLR_GR)>> Running SCAR $(CLR_NC)\n"
	make $(MKFLAGS) -C $(scar_dir)


.PHONY: clean-scar
clean-scar:							## Clean scar directory
	@printf "$(CLR_GR)>> Cleaning scar working directory$(CLR_NC)\n"
	make $(MKFLAGS) -C $(scar_dir) clean


# ======== ElfDump ========
.PHONY: elfdump
elfdump:							## Build elfdump utility
	@printf "$(CLR_GR)>> Building elfdump $(CLR_NC)\n"
	make $(MKFLAGS) -C $(elfdump_dir)


.PHONY: clean-elfdump
clean-elfdump:					    ## Clean elfdump directory
	@printf "$(CLR_GR)>> Cleaning elfdump directory [tools/elfdump/bin/*]$(CLR_NC)\n"
	make $(MKFLAGS) -C $(elfdump_dir) clean

# ======== SW libs ========
.PHONY: lib
lib:								## compile software libraries
	@printf "$(CLR_GR)>> Compiling software libraries $(CLR_NC)\n"
	make $(MKFLAGS) -C $(lib_dir) soctarget=$(soctarget)


.PHONY: clean-lib
clean-lib:							## Clean software libs
	@printf "$(CLR_GR)>> Cleaning build files for lib $(CLR_NC)\n"
	make $(MKFLAGS) -C $(lib_dir) clean


# ======== Documentation ========
.PHONY: doxy
doxy:								## Generate atomsim C++ source documentation
	@printf "$(CLR_GR)>> Generating Doxygen C++ documentation [latex & html]$(CLR_NC)\n"
	make $(MKFLAGS) -C $(doxy_dir)


.PHONY: doxy-pdf
doxy-pdf: doxy						## Generate atomsim C++ source documentation (pdf)
	@printf "$(CLR_GR)>> Generating Doxygen C++ documentation [pdf]$(CLR_NC)\n"
	make $(MKFLAGS) -C $(doxy_dir) pdf


.PHONY: clean-doxy
clean-doxy:							## Clean build files for Atomsim docs
	@printf "$(CLR_GR)>> Cleaning docs $(CLR_NC)\n"
	make $(MKFLAGS) -C $(doxy_dir) clean


# ======== clean ========
.PHONY: clean						
clean: clean-sim clean-lib					## Alias for clean-sim, clean-lib


.PHONY: clean-all					
clean-all: clean-sim clean-scar clean-elfdump clean-lib clean-doxy  ## Clean all build files