#####################################################################
###	BASIC CONFIGURATIONS

# Verilog 
topmodule = AtomRVSoC
verilog_source = rtl/$(topmodule).v

# CPP Driver
cpp_driver = AtomSim.cpp
cpp_driver_source = sim/$(cpp_driver)

# Build configuration
build_dir = build
bin_dir = $(build_dir)/bin
object_dir = $(build_dir)/obj_dir
trace_dir = $(build_dir)/trace
doc_dir = doc
doxygen_doc_dir = $(doc_dir)/doxygen
doxygen_config_file = Doxyfile

# Executable
executable_name = atomsim

########################################################################
###	ADVANCED CONFIGURATIONS
verilator_includes = -I obj_dir -I /usr/share/verilator/include -I /usr/share/verilator/include/vltstd
linking = #-lncurses
verilated_path = /usr/share/verilator/include/verilated.cpp /usr/share/verilator/include/verilated_vcd_c.cpp
gpp_flags = #-pthread
verilator_flags = -cc -Wall --relative-includes --trace

########################################################################
default: clean atomsim

.PHONY: help
help:
	@echo "make clean	: to clean compiler generated files"
	@echo "make sim		: to build the simulator"
	@echo "make		    : to do all of the above (sequentially)"


# Setup Directories
$(build_dir):
	@echo ">> Creating directory $(build_dir)"
	mkdir $(build_dir)
	@echo ">> Creating directory $(build_dir) -- done\n"


$(bin_dir):
	@echo ">> Creating directory $(bin_dir)"
	mkdir $(bin_dir)
	@echo ">> Creating directory $(bin_dir) -- done\n"

$(trace_dir):
	@echo ">> Creating directory $(trace_dir)"
	mkdir $(trace_dir)
	@echo ">> Creating directory $(trace_dir) -- done\n"

$(doc_dir):
	@echo ">> Creating directory $(doc_dir)"
	mkdir $(doc_dir)
	@echo ">> Creating directory $(doc_dir) -- done\n"

$(doxygen_doc_dir):
	@echo ">> Creating directory $(doxygen_doc_dir)"
	mkdir $(doxygen_doc_dir)
	@echo ">> Creating directory $(doxygen_doc_dir) -- done\n"

# Verilate verilog
$(object_dir)/V$(topmodule)__ALL.a: 
	@echo ">> Compiling verilog into cpp"
	verilator $(verilator_flags) $(verilog_source) --Mdir $(object_dir)
	@echo ">> Compiling verilog into cpp -- done\n"

	@echo ">> Generating shared object"
	cd $(build_dir)/obj_dir && make -f V$(topmodule).mk	
	@echo ">> Generating shared object -- done\n"

# Compile Cpp Driver
$(bin_dir)/$(executable_name): $(object_dir)/V$(topmodule)__ALL.a
	@echo ">> Compiling driver cpp file with shared object"
	g++ $(gpp_flags) $(cpp_driver_source) $(verilator_includes) $(verilated_path) $(object_dir)/V$(topmodule)__ALL.a $(linking) -o $(bin_dir)/$(executable_name)
	@echo ">> Compiling driver cpp file with shared object -- done\n"


.PHONY: atomsim
atomsim: $(build_dir) $(bin_dir) $(trace_dir) $(bin_dir)/$(executable_name)

.PHONY: doc
doc: $(doc_dir) $(doxygen_doc_dir)
	doxygen $(doxygen_config_file)
#.PHONY: sim
#sim: $(build_dir) $(bin_dir)
#	@echo ">> Compiling verilog into cpp"
#	verilator $(verilator_flags) $(verilog_source) --Mdir $(object_dir)
#	@echo ">> Compiling verilog into cpp -- done\n"
#
#	@echo ">> Generating shared object"
#	cd $(build_dir)/obj_dir && make -f V$(topmodule).mk	
#	@echo ">> Generating shared object -- done\n"
#
#	@echo ">> Compiling driver cpp file with shared object"
#	g++ $(gpp_flags) $(cpp_driver_source) $(verilator_includes) $(verilated_path) $(object_dir)/V$(topmodule)__ALL.a $(linking) -o $(executable_path)/$(executable_name)
#	@echo ">> Compiling driver cpp file with shared object -- done\n"


.PHONY: clean
clean:
	@echo ">> Clearing $(bin_dir) & $(build_dir) directories"
	rm -rf $(bin_dir)/*
	rm -rf $(build_dir)/*
	@echo ">> Clearing $(bin_dir) & $(build_dir) directories -- done\n"

.PHONY: clean-doc
clean-doc:
	@echo ">> Clearing $(doc_dir) directory"
	rm -rf $(dexygen_doc_dir)/*
	@echo ">> Clearing $(doc_dir) directory -- done\n"
