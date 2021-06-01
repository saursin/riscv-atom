TOOLCHAIN_PREFIX=riscv64-unknown-elf-
CC = gcc
CFLAGS = -nostartfiles -march=rv32i -mabi=ilp32 -T $(LDPATH)
LDPATH = ../../sw/lib/link.ld

DUMPFLAGS = -hsd

%.elf: %.S $(LDPATH)
	$(TOOLCHAIN_PREFIX)$(CC) $(CFLAGS) -o $@ $<

	
	elfdump $@
	$(TOOLCHAIN_PREFIX)objdump $(DUMPFLAGS) $@

clean:
	rm -f *.elf