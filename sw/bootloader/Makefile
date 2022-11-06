EXEC:= bootloader.elf

CFLAGS:= -mabi=ilp32 -march=rv32i -nostartfiles -ffreestanding -DTARGET_HYDROGENSOC -Os
CFLAGS+= -I $(RVATOM_LIB)/include
LFLAGS:= -L $(RVATOM_LIB)/ -T link_bootloader.lds -lcatom -Xlinker -Map $(EXEC).map

default: boot

.PHONY: boot
boot: $(EXEC)

$(EXEC): xmodem.c bootloader.c crt0.S
	riscv64-unknown-elf-gcc $(CFLAGS) -o $@ $^ $(LFLAGS)
	riscv64-unknown-elf-objdump -htd $@ > $@.objdump

app.elf:
# %.o: %.c
# 	riscv64-unknown-elf-gcc $(CFLAGS) -c -o $@ $^
# 	riscv64-unknown-elf-objdump -htd $@ > $@.objdump


.PHONY: clean
clean:
	rm -f *.o *.objdump *.map *.elf