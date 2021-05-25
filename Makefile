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
doxygen_config_file = $(doc_dir)/Doxyfile

# Executable
executable_name = atomsim

########################################################################
###	ADVANCED CONFIGURATIONS
verilator_includes = -I obj_dir -I /usr/share/verilator/include -I /usr/share/verilator/include/vltstd
linking = #-lncurses
verilated_path = /usr/share/verilator/include/verilated.cpp /usr/share/verilator/include/verilated_vcd_c.cpp
gpp_flags = -static#-pthread
verilator_flags = -cc -Wall --relative-includes --trace

########################################################################
default: clean atomsim



.PHONY: help
help:
	@echo "make clean	: to clean compiler generated files"
	@echo "make sim		: to build the simulator"
	@echo "make		    : to do all of the above (sequentially)"


.PHONY: atomsim
atomsim: directories $(bin_dir)/$(executable_name)


# create directories if necessary
directories : $(build_dir) $(bin_dir) $(trace_dir) $(doc_dir) $(doxygen_doc_file)

# Setup Directories
$(build_dir):
	mkdir $(build_dir)

$(bin_dir):
	mkdir $(bin_dir)

$(trace_dir):
	mkdir $(trace_dir)

$(doc_dir):
	mkdir $(doc_dir)

$(doxygen_doc_dir):
	mkdir $(doxygen_doc_dir)


# Verilate verilog
$(object_dir)/V$(topmodule)__ALL.a: 
	@echo ">> Compiling verilog into cpp"
	verilator $(verilator_flags) $(verilog_source) --Mdir $(object_dir)

	@echo ">> Generating shared object"
	cd $(build_dir)/obj_dir && make -f V$(topmodule).mk	


# Compile Cpp Driver
$(bin_dir)/$(executable_name): $(object_dir)/V$(topmodule)__ALL.a
	@echo ">> Compiling driver cpp file with shared object"
	g++ $(gpp_flags) $(cpp_driver_source) $(verilator_includes) $(verilated_path) $(object_dir)/V$(topmodule)__ALL.a $(linking) -o $(bin_dir)/$(executable_name)



.PHONY: documentation
documentation: $(doc_dir) $(doxygen_doc_dir)
	doxygen $(doxygen_config_file)


# Clean build files
.PHONY: clean
clean:
	rm -rf $(bin_dir)/*
	rm -rf $(build_dir)/obj_dir/*

# Clean trace directory
.PHONY: clean-trace
clean-trace:
	rm -rf $(trace_dir)/*

# Clean doc dirctory
.PHONY: clean-doc
clean-doc:
	rm -rf $(doxygen_doc_dir)/*

# Clean everything
.PHONY: clean-all
clean-all: clean clean-trace clean-doc
