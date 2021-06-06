CC = riscv64-unknown-elf-gcc
CFLAGS = -c -Wall
SRC_DIR = .
OBJ_DIR = .

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS :=  $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

default: $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o