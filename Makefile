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

# Executable
executable_name = atomsim

########################################################################
###	ADVANCED CONFIGURATIONS
verilator_includes = -I obj_dir -I /usr/share/verilator/include -I /usr/share/verilator/include/vltstd
linking = #-lncurses
verilated_path = /usr/share/verilator/include/verilated.cpp
gpp_flags = #-pthread
verilator_flags = -cc -Wall --relative-includes

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
atomsim: $(build_dir) $(bin_dir) $(bin_dir)/$(executable_name)


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
