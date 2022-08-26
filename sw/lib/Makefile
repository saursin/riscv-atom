# Bash color codes
COLOR_CYAN	    = \033[0;36m
COLOR_NC 		= \033[0m

soctarget?=None

# targets for which we don't need soctarget to be defined
#
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
ifeq ($(__check_soctarget), true)
ifeq ($(soctarget), atombones)
COMPILE_SCRIPT = compile-atombones
else
ifeq ($(soctarget), hydrogensoc)
COMPILE_SCRIPT = compile-hydrogensoc
else
$(error Invalid SoC target: $(soctarget))
endif
endif
endif


soctarget_ALLCAPS = $(shell echo $(soctarget) | tr  '[:lower:]' '[:upper:]')
OPT = true

CC = riscv64-unknown-elf-gcc
CFLAGS = -mabi=ilp32 -march=rv32i -ffreestanding -nostartfiles -nostdlib -Wall -I include -DTARGET_$(soctarget_ALLCAPS)
SRC_DIR = libcatom
OBJ_DIR = obj

ifeq ($(OPT), true)	# For optimized code
	CFLAGS+= -fdata-sections -ffunction-sections
endif

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
S_SRCS := $(wildcard $(SRC_DIR)/*.S)

C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SRCS))
S_OBJS := $(patsubst $(SRC_DIR)/%.S,$(OBJ_DIR)/%.o,$(S_SRCS))

SRCS := $(C_SRCS) $(S_SRCS)
OBJS := $(C_OBJS) $(S_OBJS)

LIB = libcatom.a


default: $(OBJ_DIR) $(LIB)			#t Build library

.PHONY: help
help:
	@echo " **** Makefile to build Atom software library **** "
	@echo ""
	@echo "Usage:"
	@echo "	$$ make soctarget=[SOCTARGET] [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s#t\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "SOCTARGETs:"
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" atombones ""
	@printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n" hydrogensoc "" 
	@echo ""


# create obj directory
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

# Create libcatom.a
$(LIB): $(OBJS)
	@printf "${COLOR_CYAN}Generating  $(LIB)$(COLOR_NC)\n"
	ar rcs $@ $^
	@printf "$(COLOR_CYAN)Build successful! $(COLOR_NC)\n"


# Compile all c files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "${COLOR_CYAN}Compiling   %-25s <-  %s$(COLOR_NC)\n" $@ $<
	$(CC) -c $(CFLAGS) $< -o $@


# Compile all s files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@printf "${COLOR_CYAN}Compiling   %-25s <-  %s$(COLOR_NC)\n" $@ $<
	$(CC) -c $(CFLAGS) $< -o $@


.PHONY: clean
clean:								#t Clean Build files
	rm -f $(OBJ_DIR)/*.o
	rm -f $(LIB)
