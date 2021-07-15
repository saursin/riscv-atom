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
# 
# Directory tree:
#	.
#	├── build
#	│   ├── bin
#	│   ├── dump
#	│   ├── cobj_dir
#	│   ├── vobj_dir
#	│   └── trace
#	├── doc
#	│   └── diagrams
#	├── rtl
#	│   ├── core
#	│   └── uncore
#	├── sim
#	│   └── include
#	├── sw
#	│   ├── examples
#	│   └── lib
#	└── test
#	    └── scar
#	        └── work
#=================================================================
# Build Configurations

build_dir 	= build
doc_dir 	= doc
rtl_dir 	= rtl
sim_dir 	= sim
tool_dir 	= tools

#=================================================================
bin_dir 		= $(build_dir)/bin
vobject_dir 	= $(build_dir)/vobj_dir
cobject_dir 	= $(build_dir)/cobj_dir
trace_dir 		= $(build_dir)/trace
dump_dir		= $(build_dir)/dump
doxygen_doc_dir = $(doc_dir)/doxygen
doxygen_config_file = $(doc_dir)/Doxyfile


# CPP Configs
CC = g++
CFLAGS = -c -Wall
LFLAGS =
INCLUDES = -I $(vobject_dir) -I /usr/share/verilator/include -I /usr/share/verilator/include/vltstd

cpp_driver = $(sim_dir)/AtomSim.cpp
sim_executable = atomsim
sim_cpp_backend = $(sim_dir)/Backend_AtomBones.hpp
Target = atombones

# Verilog Configs
VC = verilator
VFLAGS = -cc -Wall --relative-includes --trace

ifeq ($(Target), atombones)
	verilog_topmodule = AtomBones
	verilog_topmodule_file = $(rtl_dir)/AtomBones.v
	verilog_files = rtl/AtomBones.v rtl/Timescale.vh rtl/Config.vh rtl/core/AtomRV.v rtl/core/Alu.v rtl/core/Decode.v rtl/core/RegisterFile.v

	CFLAGS += -DTARGET_ATOMBONES
	
endif

#======================================================================
# Recepies
#======================================================================
default: sim elfdump
all : sim elfdump pdf-docs



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
#~	sim		:	Build atomsim simulator
.PHONY : sim
sim: buildFor directories $(bin_dir)/$(sim_executable)

buildFor:
	@echo ">> Building AtomSim for Target: $(Target)"


# Check directories
directories : $(build_dir) $(bin_dir)  $(cobject_dir) $(vobject_dir) $(trace_dir) $(dump_dir) $(doc_dir) $(doxygen_doc_dir)

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

$(doc_dir):
	mkdir $@

$(doxygen_doc_dir):
	mkdir $@

# Verilate verilog
$(vobject_dir)/V$(verilog_topmodule)__ALLsup.o $(vobject_dir)/V$(verilog_topmodule)__ALLcls.o: $(verilog_files)
	@echo ">> Compiling verilog into c++ classes..."
	$(VC) $(VFLAGS) $(verilog_topmodule_file) --Mdir $(vobject_dir)

	@echo ">> Generating shared object..."
	cd $(vobject_dir) && make -f V$(verilog_topmodule).mk

# Compile C++ files
$(cobject_dir)/atomsim.o: $(sim_dir)/AtomSim.cpp $(sim_dir)/defs.hpp $(sim_dir)/Testbench.hpp $(sim_cpp_backend)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(cobject_dir)/verilated.o: /usr/share/verilator/include/verilated.cpp 
	@echo ">> Compiling cpp include <$<>..."
	$(CC) $(CFLAGS) $^ -o $@

$(cobject_dir)/verilated_vcd.o: /usr/share/verilator/include/verilated_vcd_c.cpp
	@echo ">> Compiling cpp include <$<>..."
	$(CC) $(CFLAGS) $^ -o $@

# Link & Create executable
$(bin_dir)/$(sim_executable): $(vobject_dir)/V$(verilog_topmodule)__ALLcls.o $(vobject_dir)/V$(verilog_topmodule)__ALLsup.o $(cobject_dir)/atomsim.o $(cobject_dir)/verilated.o $(cobject_dir)/verilated_vcd.o
	@echo ">> Linking shared object and driver to create executable..."
	$(CC) $(LFLAGS) $^ -o $@




# ======== SCAR ========
#~	scar		:	verify using scar
.PHONY: scar
scar: $(bin_dir)/$(sim_executable)
	cd test/scar/ && make



# ======== ElfDump ========
#~	elfdump		:	build Elfdump
.PHONY: elfdump
elfdump: $(bin_dir)/elfdump

$(bin_dir)/elfdump: $(tool_dir)/elfdump/elfdump.cpp
	@echo ">> Building elfdump..."
	$(CC) -Wall $^ -o $@



# ======== Documentation ========
#~	docs		:	Generate doxygen documentation for atomsim source code
.PHONY: docs
docs: $(doc_dir) $(doxygen_doc_dir)
	doxygen $(doxygen_config_file)

#~	pdf-docs	:	Generate doxygen documentation for atomsim source code (in pdf fomat)
.PHONY: pdf-docs
pdf-docs: docs $(doc_dir) $(doxygen_doc_dir)
	cd doc/doxygen/latex && make
	mv doc/doxygen/latex/refman.pdf doc/Atomsim_source_documentation.pdf




# ======== clean ========
#~	clean		:	Clean all autogenerated build files
.PHONY: clean
clean:
	rm -rf $(bin_dir)/*
	rm -rf $(vobject_dir)/*
	rm -rf $(cobject_dir)/*

#~	clean-trace 	:	Clean trace directory
.PHONY: clean-trace
clean-trace:
	rm -rf $(trace_dir)/*

#~	clean-dump 	:	Clean dump directory
.PHONY: clean-dump
clean-dump:
	rm -rf $(dump_dir)/*

#~	clean-doc	: 	Clean doc dirctory
.PHONY: clean-doc
clean-doc:
	rm -rf $(doxygen_doc_dir)/*

#~	clean-all	:	Clean everything in build diectory
.PHONY: clean-all
clean-all: clean clean-trace clean-dump clean-doc