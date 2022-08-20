# Bash color codes
COLOR_CYAN	    = \033[0;36m
COLOR_NC 		= \033[0m

Target=None
Target_ALLCAPS = $(shell echo $(Target) | tr  '[:lower:]' '[:upper:]')
OPT = true

CC = riscv64-unknown-elf-gcc
CFLAGS = -mabi=ilp32 -march=rv32i -ffreestanding -nostartfiles -nostdlib -Wall -I include -DTARGET_$(Target_ALLCAPS)
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

default: $(OBJ_DIR) $(LIB)

# create obj directory
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

# Create libcatom.a
$(LIB): $(OBJS)
	@echo "$(COLOR_CYAN)Generating $(LIB)... $(COLOR_NC)"
	ar rcs $@ $^

# Compile all c files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "$(COLOR_CYAN)Compiling... [ $@	<-	$< ] $(COLOR_NC)"
	$(CC) -c $(CFLAGS) $< -o $@

# Compile all s files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@echo "$(COLOR_CYAN)Compiling... [ $@	<-	$< ] $(COLOR_NC)"
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(LIB)

.PHONY: super-clean
super-clean:
	rm -rf $(OBJ_DIR)
	rm -f $(LIB)
