####################################################
#h# ******* Makefile for Building AtomSim *******
####################################################

#v# Specify soctarget
soctarget ?= None

#v# Enable debug build
DEBUG ?= 0

#v# Enable DPI support in RTL
DPI ?= 0

include ../common.mk
####################################################

# use default soctarget if not specified
DEFAULT_SOCTARGET := atombones
ifeq ($(soctarget), None)
    soctarget := $(DEFAULT_SOCTARGET)
    $(warning No soctarget specified, building for default: $(DEFAULT_SOCTARGET))
endif

# check verilator include directory
VERILATOR_INCLUDE_PATH=$(VERILATOR_PATH)/share/verilator/include
ifeq ("$(wildcard $(VERILATOR_INCLUDE_PATH)/verilated_vcd_c.cpp)","")
    $(error Verilator include path invalid)
endif

JSONCFG:= $(RVATOM)/rtl/config/$(soctarget).json

####################################################
# Directories
RTL_DIR := ../rtl
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
VERILATED_DIR := $(BUILD_DIR)/verilated
BIN_DIR := $(BUILD_DIR)/bin
DEPDIR := $(BUILD_DIR)/.depend

# make directories during makefile-parse
$(shell mkdir -p $(OBJ_DIR) $(VERILATED_DIR) $(BIN_DIR) $(DEPDIR))

####################################################
# Verilog Configs
VC := verilator
VFLAGS := -cc -Wall --trace -D__ATOMSIM_SIMULATION__ --Mdir $(VERILATED_DIR)
VFLAGS += -DSOC_BOOTROM_INIT_FILE='"$(RVATOM)/sw/bootloader/bootloader.hex"' 
VTOPMODULE:= $(shell $(RVATOM)/scripts/cfgparse.py $(JSONCFG) --top)

####################################################
# CPP configs
CC := g++
CFLAGS :=  -std=c++14 -faligned-new -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td
LDFLAGS := -lCppLinuxSerial -lreadline
LDLIBS := -L include/CppLinuxSerial/
INCLUDES := -I . -I include/
INCLUDES += -I $(VERILATED_DIR) -I $(VERILATOR_INCLUDE_PATH) -I $(VERILATOR_INCLUDE_PATH)/vltstd

ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0
else
    CFLAGS += -O3
endif

EXE := $(BIN_DIR)/atomsim
SRCS := main.cpp atomsim.cpp vuart.cpp interactive.cpp memory.cpp util.cpp bitbang_uart.cpp
COMPILE_CPP_MACRO = $(CC) $(DEPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@

SIM_BACKEND_FILE := backend_$(soctarget).cpp
CFLAGS += -DTARGET_HEADER='"backend_$(soctarget).hpp"'

# Since we have the target specific backend file-name now, append it to list of srcs
SRCS += $(SIM_BACKEND_FILE)

####################################################
# Generate object list
OBJS_ := $(patsubst %.cpp, %.o, $(SRCS))			# change .cpp -> .o
OBJS__ := $(notdir $(OBJS_))						# extract filename, discard full path
OBJS := $(patsubst %, $(OBJ_DIR)/%, $(OBJS__))		# prefix with directory path

# generate dependency list
DEPS_ := $(patsubst %.cpp, %.Td, $(SRCS))			# change .cpp -> .Td
DEPS__ := $(notdir $(DEPS_))						# extract filename, discard full path
DEPS := $(patsubst %, $(DEPDIR)/%, $(DEPS__))		# prefix with directory path

LDFLAGS += -L build/verilated
LDLIBS += -l:V$(VTOPMODULE)__ALL.a -lverilated -lpthread

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

VSRCS := $(shell $(RVATOM)/scripts/cfgparse.py $(JSONCFG) --vsrcs)
####################################################
# Recepies

default: sim								#t# Alias for sim
sim: lib_verilated $(EXE)					#t# Build atomsim
lib_verilated: $(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a


# Verilate verilog
$(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a: $(VSRCS)
	$(call print_msg,Verilating Verilog)
	$(VC) $(VFLAGS) `$(RVATOM)/scripts/cfgparse.py $(JSONCFG) -f --tool=verilator`

	$(call print_msgt,Generating library)
	$(MAKE) -s -C $(VERILATED_DIR) -f V$(VTOPMODULE).mk > /dev/null

	$(call print_msg,Generating combined header,V$(VTOPMODULE)_headers.h)
	printf "#ifndef __V$(VTOPMODULE)_headers__\n" > $(VERILATED_DIR)/V$(VTOPMODULE)_headers.h
	printf "#define __V$(VTOPMODULE)_headers__\n" >> $(VERILATED_DIR)/V$(VTOPMODULE)_headers.h
	find $(VERILATED_DIR) -type f -name "*.h" -exec echo '#include "{}"' \; >> $(VERILATED_DIR)/V$(VTOPMODULE)_headers.h
	printf "#endif // __V$(VTOPMODULE)_headers__\n" >> $(VERILATED_DIR)/V$(VTOPMODULE)_headers.h

# Generic rule to compile Cpp Files in . (prerequisite: complete verilate step first)
$(OBJ_DIR)/%.o: %.cpp
$(OBJ_DIR)/%.o: %.cpp $(DEPDIR)/%.Td $(VERILATED_DIR)/V$(VTOPMODULE)__ALL.a
	$(call print_msgt,Compiling)
	$(COMPILE_CPP_MACRO) $<

# Generic rule to compile Cpp Files rtl/dpi
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp
$(OBJ_DIR)/%.o: $(RTL_DIR)/dpi/%.cpp $(DEPDIR)/%.Td
	$(call print_msgt,Compiling)
	$(COMPILE_CPP_MACRO) $<


# Link & Create executable
$(EXE): $(OBJS)
	$(call print_msgt,Linking)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)
	$(call print_msg,Atomsim build successful!)

.PRECIOUS: $(DEPDIR)/%.Td
$(DEPDIR)/%.Td: ;


# Lint
lint: $(VSRCS)
	@if $(VC) $(VFLAGS) --lint-only `$(RVATOM)/scripts/cfgparse.py $(JSONCFG) -f --tool=verilator`; then \
		echo "Lint check: OK"; \
	else \
		echo "Lint check: Errors Found"; \
	fi

####################################################
# Cleanup Recepies

.PHONY: clean
clean:									#t# Clean build files
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* $(VERILATED_DIR)/* $(DEPDIR)/*

-include $(DEPS)
