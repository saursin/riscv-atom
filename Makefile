#================================================================#
#      ____  _________ _______    __         __                  #
#     / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___   #
#    / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \  #
#   / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /  #
#  /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/   #
#                                                                #
#================================================================#
#
# README:
#
# 	This is a self documenting Makefile, see 'help' target for 
#	more info. Whenever adding a new taget document it a comment 
# 	that stats with a hash(#) symbol folllowed by a tilde(~) 
# 	symbol. Anything after # followed by ~ will be displayed 
#	whenever "make help: is called.
#================================================================#

# Bash color codes
COLOR_RED 		= \033[0;31m
COLOR_GREEN 	= \033[0;32m
COLOR_YELLOW 	= \033[0;33m
COLOR_NC 		= \033[0m

#=================================================================
# Build Configurations

# directories
build_dir 	= build
doc_dir 	= doc
rtl_dir 	= rtl
sim_dir 	= sim
tool_dir 	= tools

# Build directory
bin_dir 		= $(build_dir)/bin
vobject_dir 	= $(build_dir)/vobj_dir
cobject_dir 	= $(build_dir)/cobj_dir
trace_dir 		= $(build_dir)/trace
dump_dir		= $(build_dir)/dump
init_dir		= $(build_dir)/init

# Doc directory
doxygen_doc_dir = $(doc_dir)/doxygen
doxygen_config_file = $(doc_dir)/Doxyfile


# CPP Configs
CC = g++
CFLAGS = -c -Wall 
LFLAGS = -L ${sim_dir}/include/CppLinuxSerial/ -lCppLinuxSerial
INCLUDES = -I $(vobject_dir) -I /usr/share/verilator/include -I /usr/share/verilator/include/vltstd

cpp_driver = $(sim_dir)/AtomSim.cpp
sim_executable = atomsim

# Default Target
Target = atombones

# Verilog Configs
VC = verilator
VFLAGS = -cc -Wall --relative-includes --trace -D__ATOMSIM_SIMULATION__

verilog_files =  $(rtl_dir)/Timescale.vh
verilog_files += $(rtl_dir)/core/Utils.vh 
verilog_files += $(rtl_dir)/core/Defs.vh 
verilog_files += $(rtl_dir)/core/AtomRV.v 
verilog_files += $(rtl_dir)/core/Alu.v 
verilog_files += $(rtl_dir)/core/Decode.v 
verilog_files += $(rtl_dir)/core/RegisterFile.v 
verilog_files += $(rtl_dir)/core/CSR_Unit.v

# Target Specific definitions
ifeq ($(Target), atombones) 	# -----	AtomBones -----
verilog_topmodule = AtomBones
verilog_topmodule_file = $(rtl_dir)/$(verilog_topmodule).v
verilog_files += $(verilog_topmodule_file)

sim_cpp_backend = $(sim_dir)/Backend_AtomBones.hpp
CFLAGS += -DTARGET_ATOMBONES
else
ifeq ($(Target), hydrogensoc) 	# ----- HydrogenSoC -----
verilog_topmodule = HydrogenSoC
verilog_topmodule_file = $(rtl_dir)/$(verilog_topmodule).v

verilog_files += $(rtl_dir)/core/AtomRV_wb.v 
verilog_files += $(rtl_dir)/uncore/BiDirectionalIO.v
verilog_files += $(rtl_dir)/uncore/GPIO.v
verilog_files += $(rtl_dir)/uncore/DualPortRAM_wb.v
verilog_files += $(rtl_dir)/uncore/SinglePortRAM_wb.v
verilog_files += $(rtl_dir)/uncore/simpleuart_wb.v
verilog_files += $(rtl_dir)/uncore/simpleuart.v
verilog_files += $(rtl_dir)/HydrogenSoC_Config.vh
verilog_files += $(verilog_topmodule_file)

VFLAGS += -D__IMEM_INIT_FILE__='"$(RVATOM)/$(init_dir)/code.hex"'
VFLAGS += -D__DMEM_INIT_FILE__='"$(RVATOM)/$(init_dir)/data.hex"'

sim_cpp_backend = $(sim_dir)/Backend_HydrogenSoC.hpp
CFLAGS += -DTARGET_HYDROGENSOC

else
$(error Build target for AtomSim not specified; Specify a target using: make Target=<atomsim/hydrogensoc>)
endif
endif

VFLAGS += --top-module $(verilog_topmodule)

#======================================================================
# Recepies
#======================================================================
default: sim elfdump scripts libs
	@echo "\n$(COLOR_GREEN)----------------------------------------------"
	@echo "Build Succesful!"
	@echo "----------------------------------------------$(COLOR_NC)"
	@echo " 1). atomsim [$(Target)]"
	@echo " 2). elfdump tool"
	@echo " 3). scripts"
	@echo " 4). software libraries"

all : default pdf-docs
	@echo " 5). doxygen-docs in latex, html & pdf "


#======== Help ========
# See : https://swcarpentry.github.io/make-novice/08-self-doc/index.html

#~	help		:	Show help message

.PHONY : help
help : Makefile
	@echo "RISCV-Atom root Makefile"
	@echo "Usage: make [Target]"
	@echo ""
	@echo "Targets:"
	@sed -n 's/^#~//p' $<



# ======== Sim ========
#~	sim		:	Build atomsim for specified target [default: atombones]
.PHONY : sim
sim: buildFor directories $(bin_dir)/$(sim_executable)

buildFor:
	@echo "$(COLOR_GREEN)>> Checking for existing build target... $(COLOR_NC)"
	
	@if [ -f "$(bin_dir)/$(sim_executable)" ] ; then \
		CURR_TARGET=`atomsim --simtarget`;\
		echo "Existing build found for target: $${CURR_TARGET}"; \
		if [ $${CURR_TARGET} != $(Target) ] ; then \
			echo "$(COLOR_GREEN)>> Removing existing build... $(COLOR_NC)"; \
        	make clean; \
		fi; \
	else \
		echo "No existing build found"; \
	fi;

	@echo "$(COLOR_GREEN)>> Building Atomsim for $(Target)... $(COLOR_NC)"

# Check directories
directories : $(build_dir) $(bin_dir)  $(cobject_dir) $(vobject_dir) $(trace_dir) $(dump_dir) $(init_dir) $(doc_dir) $(doxygen_doc_dir)

$(build_dir):
	mkdir $@

$(bin_dir):
	mkdir $@

$(cobject_dir):
	mkdir $@

$(vobject_dir):
	mkdir $@

$(trace_dir):
	mkdir $@

$(dump_dir):
	mkdir $@

$(init_dir):
	mkdir $@

$(doc_dir):
	mkdir $@

$(doxygen_doc_dir):
	mkdir $@

# Verilate verilog
$(vobject_dir)/V$(verilog_topmodule)__ALLsup.o $(vobject_dir)/V$(verilog_topmodule)__ALLcls.o: $(verilog_files)
	@echo "$(COLOR_GREEN)Verilating RTL...$(COLOR_NC)"
	$(VC) $(VFLAGS) $(verilog_topmodule_file) --Mdir $(vobject_dir)

	@echo "$(COLOR_GREEN)Generating shared object...$(COLOR_NC)"
	cd $(vobject_dir) && make -f V$(verilog_topmodule).mk

# Compile C++ files
$(cobject_dir)/atomsim.o: $(sim_dir)/AtomSim.cpp $(sim_dir)/defs.hpp $(sim_dir)/Backend.hpp $(sim_dir)/Testbench.hpp $(sim_cpp_backend)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(cobject_dir)/verilated.o: /usr/share/verilator/include/verilated.cpp 
	@echo "$(COLOR_GREEN)Compiling cpp include [$<]...$(COLOR_NC)"
	$(CC) $(CFLAGS) $^ -o $@

$(cobject_dir)/verilated_vcd.o: /usr/share/verilator/include/verilated_vcd_c.cpp
	@echo "$(COLOR_GREEN)Compiling cpp include [$<]...$(COLOR_NC)"
	$(CC) $(CFLAGS) $^ -o $@

# Link & Create executable
$(bin_dir)/$(sim_executable): $(vobject_dir)/V$(verilog_topmodule)__ALLcls.o $(vobject_dir)/V$(verilog_topmodule)__ALLsup.o $(cobject_dir)/atomsim.o $(cobject_dir)/verilated.o $(cobject_dir)/verilated_vcd.o
	@echo "$(COLOR_GREEN)Linking shared object and driver to create executable...$(COLOR_NC)"
	$(CC) $^ -o $@ $(LFLAGS)




# ======== SCAR ========
#~	scar		:	Verify target using scar
.PHONY: scar
scar: $(bin_dir)/$(sim_executable)
	@echo "\n$(COLOR_GREEN)>> Running SCAR $(COLOR_NC)"
	cd test/scar/ && make



# ======== ElfDump ========
#~	elfdump		:	Build elfdump utility
.PHONY: elfdump
elfdump: $(bin_dir)/elfdump

$(bin_dir)/elfdump: $(tool_dir)/elfdump/elfdump.cpp
	@echo "$(COLOR_GREEN)>> Building elfdump ...$(COLOR_NC)"
	$(CC) -Wall $^ -o $@

# ======== Scripts ========
#~	scripts		:	copy scripts/* to build/bin/
.PHONY: scripts
scripts: $(build_dir) $(bin_dir)
	@echo "$(COLOR_GREEN)>> copying scripts/* to build/bin/ ...$(COLOR_NC)"
	cp scripts/* $(bin_dir)/


# ======== SW libs ========
#~	libs		:	compile software libraries
.PHONY: libs
libs: $(build_dir) $(bin_dir)
	@echo "$(COLOR_GREEN)>> Compiling software libraries ...$(COLOR_NC)"
	cd sw/lib && make Target=$(Target)

# ======== Documentation ========
#~	docs		:	Generate atomsim C++ source documentation
.PHONY: docs
docs: $(doc_dir) $(doxygen_doc_dir)
	@echo "$(COLOR_GREEN)>> Generating Doxygen C++ documentation [latex & html]...$(COLOR_NC)"
	doxygen $(doxygen_config_file)

#~	pdf-docs	:	Generate atomsim C++ source documentation (pdf)
.PHONY: pdf-docs
pdf-docs: docs $(doc_dir) $(doxygen_doc_dir)
	@echo "$(COLOR_GREEN)>> Generating Doxygen C++ documentation [pdf]...$(COLOR_NC)"
	cd doc/doxygen/latex && make
	mv doc/doxygen/latex/refman.pdf doc/Atomsim_source_documentation.pdf




# ======== clean ========
#~	clean		:	Clean atomsim build files
.PHONY: clean
clean:
	@echo "$(COLOR_GREEN)>> Cleaning build files ...$(COLOR_NC)"
	rm -rf $(bin_dir)/*
	rm -rf $(vobject_dir)/*
	rm -rf $(cobject_dir)/*

#~	clean-trace 	:	Clean trace directory
.PHONY: clean-trace
clean-trace:
	@echo "$(COLOR_GREEN)>> Cleaning trace files [$(trace_dir)/*] ...$(COLOR_NC)"
	rm -rf $(trace_dir)/*

#~	clean-dump 	:	Clean dump directory
.PHONY: clean-dump
clean-dump:
	@echo "$(COLOR_GREEN)>> Cleaning dump files [$(dump_dir)/*] ...$(COLOR_NC)"
	rm -rf $(dump_dir)/*

#~	clean-init 	:	Clean init directory
.PHONY: clean-init
clean-init:
	@echo "$(COLOR_GREEN)>> Cleaning init files [$(init_dir)/*] ...$(COLOR_NC)"
	rm -rf $(init_dir)/*


#~	clean-doc	: 	Clean doc dirctory
.PHONY: clean-doc
clean-doc:
	@echo "$(COLOR_GREEN)>> Cleaning docs [$(doxygen_doc_dir)/*] ...$(COLOR_NC)"
	rm -rf $(doxygen_doc_dir)/*

#~	clean-lib	: 	Clean lib dirctory
.PHONY: clean-lib
clean-lib:
	@echo "$(COLOR_GREEN)>> Cleaning libs [$(doxygen_doc_dir)/*] ...$(COLOR_NC)"
	cd sw/lib && make clean

#~	clean-all	:	Clean everything in build diectory
.PHONY: clean-all
clean-all: clean clean-trace clean-dump clean-init clean-doc clean-lib

#~	super-clean	:	Revert repo to untouched state (Delete build directory)
.PHONY: super-clean
super-clean:
	rm -rf $(build_dir)/
	rm -rf $(doxygen_doc_dir)/
	cd sw/lib && make super-clean
