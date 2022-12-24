##################################################
# ******** Makefile for Building libcatom ********




include ../../common.mk
##################################################
soctarget?=None

# enable this to generate one section per function, 
# this allows these sections(functions) to be removed 
# if unused when linking with user application.
# NOTE: use `-Wl,--gc-sections` while linking with 
# user application to enable gcc garbage collection
OPT = true

##################################################
# targets for which we don't need soctarget to be defined
__check_soctarget := true

ifeq ($(MAKECMDGOALS), help)
__check_soctarget := false
else 
ifeq ($(MAKECMDGOALS), clean)
__check_soctarget := false
else
ifeq ($(soctarget), None)
$(error soctarget not defined)
endif
endif
endif


# select compile script
TARGET_SPECIFIC_SRC_DIR := None
ifeq ($(__check_soctarget), true)
ifeq ($(soctarget), atombones)
TARGET_SPECIFIC_SRC_DIR := libcatom/atombones
else
ifeq ($(soctarget), hydrogensoc)
TARGET_SPECIFIC_SRC_DIR := libcatom/hydrogensoc
else
$(error Invalid SoC target: $(soctarget))
endif
endif
endif

soctarget_ALLCAPS = $(shell echo $(soctarget) | tr  '[:lower:]' '[:upper:]')

RVPREFIX := riscv64-unknown-elf
CC := $(RVPREFIX)-gcc
CFLAGS = -mabi=ilp32 -march=rv32i -ffreestanding -nostartfiles -nostdlib -Wall -I include -DTARGET_$(soctarget_ALLCAPS)
SRC_DIR = libcatom
OBJ_DIR = obj

ifeq ($(OPT), true)	# For optimized code
	CFLAGS+= -fdata-sections -ffunction-sections
endif

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

LIB = libcatom.a


default: $(OBJ_DIR) $(LIB)			#t Build library

.PHONY: help
help:
	@printf " **** Makefile to build Atom software library **** \n"
	@printf "\n"
	@printf "Usage:\n"
	@printf "	$$ make soctarget=[SOCTARGET] [TARGET]\n"
	@printf "\n"
	@printf "TARGETs:\n"
	@grep -E -h '\s#t\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@printf "\n"
	@printf "SOCTARGETs:\n"
	@printf "\t$(CLR_CY)%-20s$(CLR_NC) %s\n" atombones ""
	@printf "\t$(CLR_CY)%-20s$(CLR_NC) %s\n" hydrogensoc "" 
	@printf ""


# create obj directory
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

# Create libcatom.a
$(LIB): $(OBJS)
	@printf "${CLR_CY}Generating  $(LIB)$(CLR_NC)\n"
	ar rcs $@ $^
	$(RVPREFIX)-objdump -htd $@ > $@.objdump
	@printf "$(CLR_CY)Build successful! $(CLR_NC)\n"


COMPILE_CMD = @printf "${CLR_CY}Compiling   %-25s <-  %s$(CLR_NC)\n" $@ $<; $(CC) -c $(CFLAGS) $< -o $@



# Compile all c files
$(OBJ_DIR)/%.o: $(TARGET_SPECIFIC_SRC_DIR)/%.c
	$(COMPILE_CMD)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILE_CMD)

# Compile S files
$(OBJ_DIR)/%.o: $(TARGET_SPECIFIC_SRC_DIR)/%.S
	$(COMPILE_CMD)
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	$(COMPILE_CMD)	


.PHONY: clean
clean:								#t Clean Build files
	rm -f $(OBJ_DIR)/*.o
	rm -f $(LIB)
