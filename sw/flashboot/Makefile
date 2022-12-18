EXEC:= flashboot.elf

CFLAGS:= -mabi=ilp32 -march=rv32i -nostartfiles -ffreestanding -DTARGET_HYDROGENSOC -Os
CFLAGS+= -I $(RVATOM_LIB)/include
LFLAGS:= -L $(RVATOM_LIB)/ -T link_flashboot.lds -lcatom -Xlinker -Map $(EXEC).map -Wl,--gc-sections

default: boot

.PHONY: boot
boot: $(EXEC)

$(EXEC): main.c crt0.S
	riscv64-unknown-elf-gcc $(CFLAGS) -o $@ $^ $(LFLAGS)
	riscv64-unknown-elf-objdump -htd $@ > $@.objdump
	python3 $$RVATOM/scripts/convelf.py -t elf -j hydrogensoc.json --keep-temp $(EXEC) -c

.PHONY: clean
clean:
	rm -f *.o *.objdump *.map *.elf *.hex