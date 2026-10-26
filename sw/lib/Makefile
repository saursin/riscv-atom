#h# ***** Makefile for Building libcatom *****

#v# Compile for simulation
sim?= 1

#v# Enable debug build
debug?= 0

# enable this to generate one section per function, 
# this allows these sections(functions) to be removed 
# if unused when linking with user application.
# NOTE: use `-Wl,--gc-sections` while linking with 
# user application to enable gcc garbage collection

#v# Enable optimizable code generation
optimizable?= 1

_mk_check_env=1
_mk_check_soctarget=1
include ../../common.mk
##################################################

SRC_DIR = libcatom
OBJ_DIR = $(RVATOM_BUILD)/sw/lib/obj
$(shell mkdir -p $(OBJ_DIR))

RVPREFIX  := riscv64-unknown-elf
CFLAGS    := -march=$(RVATOM_ISA)
CFLAGS    += -mabi=$(RVATOM_ABI)
CFLAGS+= -Wall -ffreestanding -nostartfiles -nostdlib -I include
CFLAGS+= $(RVATOM_CDEFINES)
ifeq ($(optimizable), 1)	# For optimizable code
    CFLAGS+= -fdata-sections -ffunction-sections
endif

ifeq ($(sim), 1)
    CFLAGS+= -DSIM
endif

ifeq ($(debug), 1)
    CFLAGS+= -g -O0
else
    CFLAGS+= -Os
endif

TARGET_SPECIFIC_SRC_DIR:= libcatom/$(soctarget)

# Common sources
C_SRCS := $(wildcard $(SRC_DIR)/*.c)
S_SRCS := $(wildcard $(SRC_DIR)/*.S)

C_SRCS += $(wildcard $(TARGET_SPECIFIC_SRC_DIR)/*.c)
S_SRCS += $(wildcard $(TARGET_SPECIFIC_SRC_DIR)/*.S)

# Generate objects
C_OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(C_SRCS)))
S_OBJS := $(patsubst %.S,$(OBJ_DIR)/%.o,$(notdir $(S_SRCS)))

SRCS := $(C_SRCS) $(S_SRCS)
OBJS := $(C_OBJS) $(S_OBJS)

# Output library
LIB = $(RVATOM_BUILD)/sw/lib/libcatom.a

default: build
build: $(LIB)						#t# Build libcatom

# Create libcatom.a
$(LIB): $(OBJS)
	$(call print_msgt,Building)
	ar rcs $@ $^
	$(RVPREFIX)-objdump -htd $@ > $(basename $@).lst

# Compile all c files
$(OBJ_DIR)/%.o: $(TARGET_SPECIFIC_SRC_DIR)/%.c $(RVATOM_VARS_MK)
	$(call print_msgt,Compiling)
	$(RVPREFIX)-gcc -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(RVATOM_VARS_MK)
	$(call print_msgt,Compiling)
	$(RVPREFIX)-gcc -c $(CFLAGS) $< -o $@

	
# Compile S files
$(OBJ_DIR)/%.o: $(TARGET_SPECIFIC_SRC_DIR)/%.S $(RVATOM_VARS_MK)
	$(call print_msgt,Compiling)
	$(RVPREFIX)-gcc -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S $(RVATOM_VARS_MK)
	$(call print_msgt,Compiling)
	$(RVPREFIX)-gcc -c $(CFLAGS) $< -o $@

	
.PHONY: clean
clean:								#t# Clean Build files
	rm -rf $(OBJ_DIR)/* $(LIB) $(basename $(LIB)).lst

