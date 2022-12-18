#h *** Makefile for Building AtomSim ***

####################################################
# override these with CLI

soctarget ?= atombones
# Debug = 1
# DPI = 1
# NO_COLOR = 1


# Define bash color codes
ifndef NO_COLOR
COLOR_RED 		:= \033[0;31m
COLOR_GREEN 	:= \033[0;32m
COLOR_YELLOW 	:= \033[0;33m
COLOR_CYAN 	    := \033[0;36m
COLOR_NC 		:= \033[0m
endif


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
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(VERILATED_DIR) $(DEPDIR) $(RUN_DIR))


####################################################
# Verilog Configs

VC := verilator
VFLAGS := -cc -Wall --trace -D__ATOMSIM_SIMULATION__ --relative-includes
VFLAGS += -I$(RTL_DIR) -I$(RTL_DIR)/common -I$(RTL_DIR)/core
VFLAGS += --Mdir $(VERILATED_DIR)

# This should point to verilator include directory
VERILATOR_INCLUDE_PATH := /tools/vlsi/verilator/share/verilator/include

ifeq ("$(wildcard $(VERILATOR_INCLUDE_PATH)/verilated_vcd_c.cpp)","")
$(error Verilator include path invalid; Set correct Verilator include path in sim/Makefile)
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
# SOCtarget Specific definitions

ifeq ($(soctarget), atombones) 	    # =========================[ AtomBones ]

VTOPMODULE := AtomBones
VTOPMODULE_FILE := $(RTL_DIR)/$(VTOPMODULE).v
SIM_BACKEND_FILE := backend_atombones.cpp
CFLAGS += -DTARGET_HEADER='"backend_atombones.hpp"'


else
ifeq ($(soctarget), hydrogensoc) 	# =========================[ HydrogenSoC ]

VTOPMODULE := HydrogenSoC
VTOPMODULE_FILE := $(RTL_DIR)/$(VTOPMODULE).v

# core wrapper
VSRCS += $(RTL_DIR)/core/AtomRV_wb.v 

# arbiter
VSRCS += $(RTL_DIR)/uncore/wishbone/arbiter.v
VSRCS += $(RTL_DIR)/uncore/wishbone/arbiter3_wb.v
VSRCS += $(RTL_DIR)/uncore/wishbone/priority_encoder.v

# periphs
VSRCS += $(RTL_DIR)/uncore/gpio/IOBuf.v
VSRCS += $(RTL_DIR)/uncore/gpio/GPIO.v
VSRCS += $(RTL_DIR)/uncore/mem/DualPortRAM_wb.v
VSRCS += $(RTL_DIR)/uncore/mem/SinglePortRAM_wb.v
VSRCS += $(RTL_DIR)/uncore/uart/UART.v
VSRCS += $(RTL_DIR)/uncore/uart/UART_core.v

VFLAGS += -D__ROM_INIT_FILE__='"$(RVATOM)/sim/$(RUN_DIR)/rom.hex"'

SRCS += bitbang_uart.cpp
SIM_BACKEND_FILE := backend_hydrogensoc.cpp
CFLAGS += -DTARGET_HEADER='"backend_hydrogensoc.hpp"'

else
$(error Build target for AtomSim not specified; Specify a target using: make soctarget=<atomsim/hydrogensoc>)
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

# Append verilator objects	(verilated.o & verilated_vcd_c.o)
OBJS += $(OBJ_DIR)/verilated.o $(OBJ_DIR)/verilated_vcd_c.o		

LDFLAGS += -L build/verilated
LDLIBS += -l:V$(VTOPMODULE)__ALL.a

# Append DPI objects & add DPI flags 	(if DPI is defined)
ifdef DPI
OBJS += $(OBJ_DIR)/util_dpi.o
VFLAGS += -DDPI_LOGGER -DLOG_RVATOM_JUMP
endif


# Append debugigng flags
ifdef DEBUG
	CFLAGS += -g -O0 -DDEBUG_LEVEL=$(DEBUG)
else    
	CFLAGS += -O0 -DNDEBUG
endif



####################################################
# Recepies

default: sim								#t Alias for sim
sim: lib_verilated $(EXE)					#t Build atomsim
lib_verilated: $(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a


# Verilate verilog
$(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a: $(VTOPMODULE_FILE) $(VSRCS)
	@printf "$(COLOR_CYAN)Verilating Verilog files$(COLOR_NC)\n"
	$(VC) $(VFLAGS) $^

	@printf "$(COLOR_CYAN)Generating shared object$(COLOR_NC)\n"
	make -C $(VERILATED_DIR) -f V$(VTOPMODULE).mk


# Generic rule to compile Cpp Files in .
$(OBJ_DIR)/%.o: %.cpp
$(OBJ_DIR)/%.o: %.cpp $(DEPDIR)/%.Td
	@printf "$(COLOR_CYAN)Compiling   %-35s <-  %s$(COLOR_NC)\n" $@ $<
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files in VERILATOR_INCLUDE_PATH (verilated.cpp & verilated.vcd_c.cpp)
$(OBJ_DIR)/%.o: $(VERILATOR_INCLUDE_PATH)/%.cpp
$(OBJ_DIR)/%.o: $(VERILATOR_INCLUDE_PATH)/%.cpp $(DEPDIR)/%.Td
	@printf "$(COLOR_CYAN)Compiling   %-35s <-  %s$(COLOR_NC)\n" $@ $<
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files rtl/dpi
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp $(DEPDIR)/%.Td
	@printf "$(COLOR_CYAN)Compiling   %-35s <-  %s$(COLOR_NC)\n" $@ $<
	$(COMPILE_CPP_MACRO) $<


# Link & Create executable
$(EXE): $(OBJS)
	@printf "$(COLOR_CYAN)Linking shared object and driver to create executable $(COLOR_NC)\n"
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

	@printf "$(COLOR_CYAN)Build successful! $(COLOR_NC)\n"

.PRECIOUS: $(DEPDIR)/%.Td
$(DEPDIR)/%.Td: ;



####################################################
# Cleanup & Util Recepies

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
	@egrep -h '\s#t\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile


# Extra info footer

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
