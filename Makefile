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

# Bash color codes
COLOR_RED 		= \033[0;31m
COLOR_GREEN 	= \033[0;32m
COLOR_YELLOW 	= \033[0;33m
COLOR_CYAN 	    = \033[0;36m
COLOR_NC 		= \033[0m

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
	@echo "\n$(COLOR_GREEN)=============================="
	@echo "       Build Succesful!"
	@echo "==============================$(COLOR_NC)"
	@echo " - atomsim [$(soctarget)]"
	@echo " - software libraries"

all : doxy-pdf default				## Build default with docs
	@echo " - doxygen-docs in latex, html & pdf "


#======== Help ========
.PHONY : help
help : Makefile						## Show help message
	@echo "*** RISC-V Atom root Makefile ***"
	@echo "Usage:"
	@echo "	$$ make soctarget=[SOCTARGET] [TARGET]"
	@echo ""
	
	@echo "SOCTARGETs:"
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" "atombones" "A barebone Atom based SoC with simulated memories"
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" "hydrogensoc" "A minimal Atom based SoC"

	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile


# ======== AtomSim ========
.PHONY : sim
sim:                       			## Build atomsim for given target [default: atombones]
	@echo "$(COLOR_GREEN)>> Building Atomsim for soctarget=$(soctarget) $(COLOR_NC)"
	make $(MKFLAGS) -C $(sim_dir) soctarget=$(soctarget) DEBUG=1


.PHONY: clean-sim
clean-sim:							## Clean atomsim build files
	@echo "$(COLOR_GREEN)>> Cleaning atomsim build files $(COLOR_NC)"
	make $(MKFLAGS) -C $(sim_dir) clean


# ======== SCAR ========
.PHONY: scar     			
scar: $(bin_dir)/$(sim_executable)	## Verify target using scar
	@echo "$(COLOR_GREEN)>> Running SCAR $(COLOR_NC)"
	make $(MKFLAGS) -C $(scar_dir)


.PHONY: clean-scar
clean-scar:							## Clean scar directory
	@echo "$(COLOR_GREEN)>> Cleaning scar working directory$(COLOR_NC)"
	make $(MKFLAGS) -C $(scar_dir) clean


# ======== ElfDump ========
.PHONY: elfdump
elfdump:							## Build elfdump utility
	@echo "$(COLOR_GREEN)>> Building elfdump $(COLOR_NC)"
	make $(MKFLAGS) -C $(elfdump_dir)


.PHONY: clean-elfdump
clean-elfdump:					    ## Clean elfdump directory
	@echo "$(COLOR_GREEN)>> Cleaning elfdump directory [tools/elfdump/bin/*]$(COLOR_NC)"
	make $(MKFLAGS) -C $(elfdump_dir) clean

# ======== SW libs ========
.PHONY: lib
lib:								## compile software libraries
	@echo "$(COLOR_GREEN)>> Compiling software libraries $(COLOR_NC)"
	make $(MKFLAGS) -C $(lib_dir) soctarget=$(soctarget)


.PHONY: clean-lib
clean-lib:							## Clean software libs
	@echo "$(COLOR_GREEN)>> Cleaning build files for lib $(COLOR_NC)"
	make $(MKFLAGS) -C $(lib_dir) clean


# ======== Documentation ========
.PHONY: doxy
doxy:								## Generate atomsim C++ source documentation
	@echo "$(COLOR_GREEN)>> Generating Doxygen C++ documentation [latex & html]$(COLOR_NC)"
	make $(MKFLAGS) -C $(doxy_dir)


.PHONY: doxy-pdf
doxy-pdf: doxy						## Generate atomsim C++ source documentation (pdf)
	@echo "$(COLOR_GREEN)>> Generating Doxygen C++ documentation [pdf]$(COLOR_NC)"
	make $(MKFLAGS) -C $(doxy_dir) pdf


.PHONY: clean-doxy
clean-doxy:							## Clean build files for Atomsim docs
	@echo "$(COLOR_GREEN)>> Cleaning docs $(COLOR_NC)"
	make $(MKFLAGS) -C $(doxy_dir) clean


# ======== clean ========
.PHONY: clean						
clean: clean-sim clean-lib					## Alias for clean-sim, clean-lib


.PHONY: clean-all					
clean-all: clean-sim clean-scar clean-elfdump clean-lib clean-doxy  ## Clean all build files