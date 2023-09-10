####################################################
#h ********* Makefile for Building AtomSim *********
####################################################

# To configure build, override the following 
# vaiables from command line while calling make, 
# like so:
#	$ make soctarget=hydrogensoc

soctarget ?= None
DEBUG ?= 0
DPI ?= 0

# Uncomment/define while calling make to disable colors
# NO_COLOR = 1

include ../common.mk


####################################################
# Directories

RTL_DIR := ../rtl
BUILD_DIR := build

OBJ_DIR := $(BUILD_DIR)/obj
VERILATED_DIR := $(BUILD_DIR)/verilated
BIN_DIR := $(BUILD_DIR)/bin
DEPDIR := $(BUILD_DIR)/.depend

RUN_DIR := run

# make directories during makefile-parse
$(shell mkdir -p $(OBJ_DIR) $(VERILATED_DIR) $(BIN_DIR) $(DEPDIR) $(RUN_DIR))

####################################################
# Verilog Configs

VC := verilator
VFLAGS := -cc -Wall --trace -D__ATOMSIM_SIMULATION__ --relative-includes		# Fixme: Simplify this macro name
VFLAGS += -I$(RTL_DIR) -I$(RTL_DIR)/common -I$(RTL_DIR)/core
VFLAGS += --Mdir $(VERILATED_DIR)

# check verilator include directory
VERILATOR_INCLUDE_PATH=$(VERILATOR_PATH)/share/verilator/include
ifeq ("$(wildcard $(VERILATOR_INCLUDE_PATH)/verilated_vcd_c.cpp)","")
    $(error Verilator include path invalid)
endif

# Core files
VSRCS := $(RTL_DIR)/core/AtomRV.v
VSRCS += $(RTL_DIR)/core/Alu.v
VSRCS += $(RTL_DIR)/core/Decode.v 
VSRCS += $(RTL_DIR)/core/RegisterFile.v 
VSRCS += $(RTL_DIR)/core/CSR_Unit.v


####################################################
# CPP configs

CC := g++
CFLAGS :=  -std=c++11 -faligned-new -Wall -Wextra #-pedantic
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td
LDFLAGS := -lCppLinuxSerial
LDLIBS := -L include/CppLinuxSerial/
INCLUDES := -I . -I include/
INCLUDES += -I $(VERILATED_DIR) -I $(VERILATOR_INCLUDE_PATH) -I $(VERILATOR_INCLUDE_PATH)/vltstd

EXE := $(BIN_DIR)/atomsim
SRCS := main.cpp atomsim.cpp vuart.cpp interactive.cpp
SRCS += memory.cpp util.cpp

# Compile Macro
COMPILE_CPP_MACRO = $(CC) $(DEPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@


####################################################
# Soctarget Specific definitions

# use default soctarget if not specified
DEFAULT_SOCTARGET := atombones

ifeq ($(soctarget), None)
    soctarget := $(DEFAULT_SOCTARGET)
    $(warning No soctarget specified, building for default: $(DEFAULT_SOCTARGET))
endif

# -------------------- AtomBones --------------------
ifeq ($(soctarget), atombones)
    VTOPMODULE := AtomBones
    VTOPMODULE_FILE := $(RTL_DIR)/soc/atombones/$(VTOPMODULE).v
    SIM_BACKEND_FILE := backend_atombones.cpp
    CFLAGS += -DTARGET_HEADER='"backend_atombones.hpp"'

else
# ------------------- HydrogenSoC -------------------
ifeq ($(soctarget), hydrogensoc)

    VTOPMODULE := HydrogenSoC
    VTOPMODULE_FILE := $(RTL_DIR)/soc/hydrogensoc/$(VTOPMODULE).v

    # core wrapper
    VSRCS += $(RTL_DIR)/core/AtomRV_wb.v 

    # arbiter
    VSRCS += $(RTL_DIR)/uncore/wishbone/arbiter.v
    VSRCS += $(RTL_DIR)/uncore/wishbone/arbiter2_wb.v
    VSRCS += $(RTL_DIR)/uncore/wishbone/arbiter3_wb.v
    VSRCS += $(RTL_DIR)/uncore/wishbone/priority_encoder.v

    # periphs
    VSRCS += $(RTL_DIR)/uncore/gpio/IOBuf.v
    VSRCS += $(RTL_DIR)/uncore/gpio/GPIO.v
    VSRCS += $(RTL_DIR)/uncore/mem/DualPortRAM_wb.v
    VSRCS += $(RTL_DIR)/uncore/mem/SinglePortRAM_wb.v
    VSRCS += $(RTL_DIR)/uncore/uart/UART.v
    VSRCS += $(RTL_DIR)/uncore/uart/UART_core.v
    VSRCS += $(RTL_DIR)/uncore/spi/SPI_wb.v
    VSRCS += $(RTL_DIR)/uncore/spi/SPI_core.v
    VFLAGS += -D__ROM_INIT_FILE__='"$(RVATOM)/sim/$(RUN_DIR)/rom.hex"'

    # cpp
    SRCS += bitbang_uart.cpp
    SIM_BACKEND_FILE := backend_hydrogensoc.cpp
    CFLAGS += -DTARGET_HEADER='"backend_hydrogensoc.hpp"'

else
$(error soctarget not specified; See make help)
endif
endif


# Since we have the target rspecific backend file-name now, append it to list of srcs
SRCS += $(SIM_BACKEND_FILE)

# Since we have the target rspecific topmodule-name now, specify topmodule name in VFLAGS
VFLAGS += --top-module $(VTOPMODULE)

# Generate object list
OBJS_ := $(patsubst %.cpp, %.o, $(SRCS))			# change .cpp -> .o
OBJS__ := $(notdir $(OBJS_))						# extract filename, discard full path
OBJS := $(patsubst %, $(OBJ_DIR)/%, $(OBJS__))		# prefix with directory path

# generate dependency list
DEPS_ := $(patsubst %.cpp, %.Td, $(SRCS))			# change .cpp -> .Td
DEPS__ := $(notdir $(DEPS_))						# extract filename, discard full path
DEPS := $(patsubst %, $(DEPDIR)/%, $(DEPS__))		# prefix with directory path

# Append verilator objects	(verilated.o, verilated_vcd_c.o, verilated_threads.o)
OBJS += $(OBJ_DIR)/verilated.o $(OBJ_DIR)/verilated_vcd_c.o	$(OBJ_DIR)/verilated_threads.o

LDFLAGS += -L build/verilated
LDLIBS += -l:V$(VTOPMODULE)__ALL.a -lpthread


# Append DPI objects & add DPI flags 	(if DPI is defined)
ifeq ($(DPI), 1)
    OBJS += $(OBJ_DIR)/util_dpi.o
    VFLAGS += -DDPI_LOGGER -DLOG_RVATOM_JUMP
endif

# Append debugigng flags
ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0 -DDEBUG_LEVEL=$(DEBUG)
else    
    CFLAGS += -O3 -DNDEBUG
endif


####################################################
# Recepies

default: sim								#t Alias for sim
sim: lib_verilated $(EXE)					#t Build atomsim
lib_verilated: $(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a


# Verilate verilog
$(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a: $(VTOPMODULE_FILE) $(VSRCS)
	@printf "$(CLR_CY)> Verilating Verilog files$(CLR_NC)\n"
	$(VC) $(VFLAGS) $^

	@printf "$(CLR_CY)> Generating shared object$(CLR_NC)\n"
	make -s -C $(VERILATED_DIR) -f V$(VTOPMODULE).mk


# Generic rule to compile Cpp Files in .
$(OBJ_DIR)/%.o: %.cpp
$(OBJ_DIR)/%.o: %.cpp $(DEPDIR)/%.Td
	@printf "$(CLR_CY)> Compiling   %-35s <-  %s$(CLR_NC)\n" $@ $<
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files in VERILATOR_INCLUDE_PATH (verilated.cpp & verilated.vcd_c.cpp)
$(OBJ_DIR)/%.o: $(VERILATOR_INCLUDE_PATH)/%.cpp
$(OBJ_DIR)/%.o: $(VERILATOR_INCLUDE_PATH)/%.cpp $(DEPDIR)/%.Td
	@printf "$(CLR_CY)> Compiling   %-35s <-  %s$(CLR_NC)\n" $@ $<
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files rtl/dpi
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp $(DEPDIR)/%.Td
	@printf "$(CLR_CY)> Compiling   %-35s <-  %s$(CLR_NC)\n" $@ $<
	$(COMPILE_CPP_MACRO) $<


# Link & Create executable
$(EXE): $(OBJS)
	@printf "$(CLR_CY)> Linking shared object and driver to create executable $(CLR_NC)\n"
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)
	@printf "$(CLR_CY)> Build successful!  [soctarget: $(soctarget)] $(CLR_NC)\n"

.PRECIOUS: $(DEPDIR)/%.Td
$(DEPDIR)/%.Td: ;



####################################################
# Util & Cleanup Recepies

.PHONY: preprocess_verilog
preprocess_verilog: $(VTOPMODULE_FILE) $(VSRCS)     #t Preprocess verilog and dump output into a single verilog file
	$(VC) $(VFLAGS) -E $^ > $(soctarget)_dump.v

.PHONY: lint_verilog
lint_verilog: $(VTOPMODULE_FILE) $(VSRCS)           #t Lint-check verilog code
	$(VC) $(VFLAGS) --lint-only $^

.PHONY: clean
clean:									#t Clean build files
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* $(VERILATED_DIR)/* $(DEPDIR)/*


.PHONY: help
help: 									#t Show help
	@sed -n 's/^#h //p' Makefile
	@echo ""
	@echo "Usage:"
	@echo "	$$ make soctarget=[SOCTARGET] [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@grep -E -h '\s#t\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile


# Extra to be printed in make help
#f 
#f SOCTARGETs:
#f 	atombones
#f 	hydrogensoc
#f 
#f Note:
#f  - Use DEBUG=1/2/3 to build debug binary (also sets debug level)
#f  - Use DPI=1 to enable DPI support in RTL
#f  - Use NO_COLOR=1 to disable coloured output
####################################################
-include $(DEPS)
