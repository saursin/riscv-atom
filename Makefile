#====================================================================#
#h#.     ____  _________ _______    __         __                 
#h#.    / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___  
#h#.   / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \ 
#h#.  / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / / 
#h#. /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/  
#h#                                                               
#====================================================================#
# README:
#	This is the root makefile for the RISC-V Atom project. It 
#	provides the convenience of building sim, tools, lib, etc from 
#	the root of the project directory. see `$ make help` for info 
#	on how to usage instructions and available targets
#
#====================================================================#
#h# ***** RISC-V Atom Root Makefile *****

#v# Specify soctarget
soctarget?= atombones

#v# Build for simulation
sim?=1

#v# Enable debug build of atomsim
debug?=0


# Flags to the makefiles
MKFLAGS := -s 

_mk_check_env:=1
include common.mk
#######################################################################

# Directories
sim_dir  		:= sim
scar_dir 		:= test/scar
elfdump_dir 	:= tools/elfdump
bootloader_dir 	:= sw/bootloader
lib_dir			:= sw/lib
doxy_dir		:= sim/docs

#======================================================================
# Recepies
#======================================================================

default: sim lib boot  				#t# Build atomsim, libcatom and bootloader
	@printf "\n$(CLR_GR)==============================\n"
	@printf "       Build Succesful!\n"
	@printf "==============================$(CLR_NC)\n"
	@printf " soctarget: $(soctarget), sim: $(sim), debug: $(debug)\n"

all : doxy-pdf default				#t# Build default with docs

# ======== AtomSim ========
.PHONY : sim
sim: boot                      		#t# Build atomsim for given soctarget
	$(call print_msg_root,Building AtomSim)
	$(MAKE) $(MKFLAGS) -C $(sim_dir) soctarget=$(soctarget) DEBUG=$(debug)


.PHONY: clean-sim
clean-sim:							#t# Clean atomsim build files
	$(call print_msg_root,Cleaning AtomSim build files)
	$(MAKE) $(MKFLAGS) -C $(sim_dir)  soctarget=$(soctarget) clean


.PHONY: test
test: sim lib						#t# Test the build using banner example
	$(call print_msg_root,Running example on Atomsim)
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/examples soctarget=$(soctarget) ex=banner sim=1 clean compile run


# ======== Bootloader ========
.PHONY : boot
boot: lib                     		#t# Build bootloader for given target
	$(call print_msg_root,Building bootloader)
	$(MAKE) $(MKFLAGS) -C $(bootloader_dir) soctarget=$(soctarget) sim=$(sim)


.PHONY: clean-boot
clean-boot:							#t# Clean bootloader build files
	$(call print_msg_root,Cleaning bootloader build files)
	$(MAKE) $(MKFLAGS) -C $(bootloader_dir) soctarget=$(soctarget) clean

# ======== Lint ========
.PHONY: lint
lint:								#t# Run lint on the project
	$(call print_msg_root,Running Lint)
	$(MAKE) $(MKFLAGS) -C $(sim_dir) lint

# ======== SCAR ========
.PHONY: scar     			
scar: sim 		 					#t# Verify target using scar
	$(call print_msg_root,Running SCAR)
	$(MAKE) $(MKFLAGS) -C $(scar_dir)


.PHONY: clean-scar
clean-scar:							#t# Clean scar directory
	$(call print_msg_root,Cleaning SCAR working directory)
	$(MAKE) $(MKFLAGS) -C $(scar_dir) clean


# ======== ElfDump ========
.PHONY: elfdump
elfdump:							#t# Build elfdump utility
	$(call print_msg_root,Building ELFDump)
	$(MAKE) $(MKFLAGS) -C $(elfdump_dir)


.PHONY: clean-elfdump
clean-elfdump:					    #t# Clean elfdump directory
	$(call print_msg_root,Cleaning ELFDump build files)
	$(MAKE) $(MKFLAGS) -C $(elfdump_dir) clean

# ======== SW libs ========
.PHONY: lib
lib:								#t# compile software libraries
	$(call print_msg_root,Building libcatom)
	$(MAKE) $(MKFLAGS) -C $(lib_dir) soctarget=$(soctarget) sim=$(sim)


.PHONY: clean-lib
clean-lib:							#t# Clean software libs
	$(call print_msg_root,Cleaning libcatom build files)
	$(MAKE) $(MKFLAGS) -C $(lib_dir) clean


# ======== Documentation ========
.PHONY: doxy
doxy:								#t# Generate atomsim C++ source documentation
	$(call print_msg_root,Generating docs for AtomSim,LATEX & HTML)
	$(MAKE) $(MKFLAGS) -C $(doxy_dir)


.PHONY: doxy-pdf
doxy-pdf: doxy						#t# Generate atomsim C++ source documentation (pdf)
	$(call print_msg_root,Generating docs for AtomSim,PDF)
	$(MAKE) $(MKFLAGS) -C $(doxy_dir) pdf


.PHONY: clean-doxy
clean-doxy:							#t# Clean build files for Atomsim docs
	$(call print_msg_root,Cleaning docs build files)
	$(MAKE) $(MKFLAGS) -C $(doxy_dir) clean


# ======== clean ========
.PHONY: clean						
clean: clean-sim clean-boot clean-lib					#t# Alias for clean-sim, clean-lib


.PHONY: clean-all					
clean-all: clean-sim clean-boot clean-scar clean-elfdump clean-lib clean-doxy  #t# Clean all build files