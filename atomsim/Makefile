#h *** Makefile for Building AtomSim ***

# This is a self-documenting makefile
# Use "#h comment..." to add a line to header
# Use "#f comment..." to add a line to footer
# Use "#t comment..." to add info about target


####################################################
# ovveride these with CLI

Target ?= atombones
# Debug = 1
# DPI = 1
# NO_COLOR = 1


# Define bash color codes
ifndef NO_COLOR
COLOR_RED 		:= \033[0;31m
COLOR_GREEN 	:= \033[0;32m
COLOR_YELLOW 	:= \033[0;33m
COLOR_NC 		:= \033[0m
endif


####################################################
# Directories

RTL_DIR := ../rtl
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
VERILATED_DIR := $(BUILD_DIR)/verilated
BIN_DIR := $(BUILD_DIR)/bin
INIT_DIR := $(BUILD_DIR)/init
DEPDIR := $(BUILD_DIR)/.depend

# make directories during makefile-parse
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(VERILATED_DIR) $(INIT_DIR) $(DEPDIR))


####################################################
# Verilog Configs

VC := verilator
VFLAGS := -cc -Wall --relative-includes --trace -D__ATOMSIM_SIMULATION__

# This should point to verilator include directory
VERILATOR_INCLUDE_PATH := /usr/local/share/verilator/include

# Core files
VSRCS =  $(RTL_DIR)/Timescale.vh
VSRCS += $(RTL_DIR)/core/Utils.vh 
VSRCS += $(RTL_DIR)/core/Defs.vh 
VSRCS += $(RTL_DIR)/core/AtomRV.v 
VSRCS += $(RTL_DIR)/core/Alu.v 
VSRCS += $(RTL_DIR)/core/Decode.v 
VSRCS += $(RTL_DIR)/core/RegisterFile.v 
VSRCS += $(RTL_DIR)/core/CSR_Unit.v

VFLAGS += -D__IMEM_INIT_FILE__='"$(INIT_DIR)/code.hex"'
VFLAGS += -D__DMEM_INIT_FILE__='"$(INIT_DIR)/data.hex"'

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
# Target Specific definitions

ifeq ($(Target), atombones) 	# =========================[ AtomBones ]

VTOPMODULE := AtomBones
VTOPMODULE_FILE := $(RTL_DIR)/$(VTOPMODULE).v
SIM_BACKEND_FILE := backend_atombones.cpp
CFLAGS += -DTARGET_HEADER='"backend_atombones.hpp"'


else
ifeq ($(Target), hydrogensoc) 	# =========================[ HydrogenSoC ]

VTOPMODULE := HydrogenSoC
VTOPMODULE_FILE := $(RTL_DIR)/$(VTOPMODULE).v

VSRCS += $(RTL_DIR)/core/AtomRV_wb.v 
VSRCS += $(RTL_DIR)/uncore/BiDirectionalIO.v
VSRCS += $(RTL_DIR)/uncore/GPIO.v
VSRCS += $(RTL_DIR)/uncore/DualPortRAM_wb.v
VSRCS += $(RTL_DIR)/uncore/SinglePortRAM_wb.v
VSRCS += $(RTL_DIR)/uncore/simpleuart_wb.v
VSRCS += $(RTL_DIR)/uncore/simpleuart.v
VSRCS += $(RTL_DIR)/HydrogenSoC_Config.vh

# VFLAGS += -D__IMEM_INIT_FILE__='"$(RVATOM)/$(init_dir)/code.hex"'
# VFLAGS += -D__DMEM_INIT_FILE__='"$(RVATOM)/$(init_dir)/data.hex"'

SIM_BACKEND_FILE := backend_hydrogensoc.cpp
CFLAGS += -DTARGET_HEADER='"backend_hydrogensoc.hpp"'

else
$(error Build target for AtomSim not specified; Specify a target using: make Target=<atomsim/hydrogensoc>)
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
LDLIBS += -l:V${VTOPMODULE}__ALL.a

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

default: sim
sim: lib_verilated $(EXE)	# Build atomsim
lib_verilated: $(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a


# Verilate verilog
$(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a: $(VTOPMODULE_FILE) $(VSRCS)
	@echo "$(COLOR_GREEN)Verilating RTL...$(COLOR_NC)"
	$(VC) $(VFLAGS) $< --Mdir $(VERILATED_DIR)

	@echo "$(COLOR_GREEN)Generating shared object... $(COLOR_NC)"
	make -C $(VERILATED_DIR) -f V$(VTOPMODULE).mk


# Generic rule to compile Cpp Files in .
$(OBJ_DIR)/%.o: %.cpp
$(OBJ_DIR)/%.o: %.cpp $(DEPDIR)/%.Td
	@echo "$(COLOR_GREEN)Compiling...	[$@	<-	$<] $(COLOR_NC)(using r1)"
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files in VERILATOR_INCLUDE_PATH (verilated.cpp & verilated.vcd_c.cpp)
$(OBJ_DIR)/%.o: $(VERILATOR_INCLUDE_PATH)/%.cpp
$(OBJ_DIR)/%.o: $(VERILATOR_INCLUDE_PATH)/%.cpp $(DEPDIR)/%.Td
	@echo "$(COLOR_GREEN)Compiling...	[$@	<-	$<] $(COLOR_NC)(using r3)"
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files rtl/dpi
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp $(DEPDIR)/%.Td
	@echo "$(COLOR_GREEN)Compiling...	[$@	<-	$<] $(COLOR_NC)(using r4)"
	$(COMPILE_CPP_MACRO) $<


# Link & Create executable
$(EXE): $(OBJS)
	@echo "$(COLOR_GREEN)Linking shared object and driver to create executable...$(COLOR_NC)"
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)


.PRECIOUS: $(DEPDIR)/%.Td
$(DEPDIR)/%.Td: ;



####################################################
# Cleanup & Util Recepies

.PHONY: clean
clean:									#t Clean build files
	rm -rf $(OBJ_DIR)/* $(VERILATED_DIR)/* $(BIN_DIR)/* $(DEPDIR)/*


.PHONY: super-clean
super-clean:							#t Revert to untouched state (clean build dir)
	rm -rf $(BUILD_DIR)/


.PHONY: help
help: 									#t Show help
	@sed -n 's/^#h //p' Makefile
	@echo ""
	@echo "Usage:"
	@echo "	$$ make Target=<SIMTARGET> (TARGET)"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s#t\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile


# Extra info footer

#f Default TARGET: sim
#f 
#f SIMTARGETs:
#f 	atombones
#f 	hydrogensoc
#f 
#f Note:
#f  - Use DEBUG=1/2/3  to build debug binary (also sets debug level)
#f  - Use DPI=1 to enable DPI support in RTL
#f  - Use NO_COLOR=1 to disable coloured output

####################################################
-include $(DEPS)
