EXEC:= bootloader

RVPREFIX:= riscv64-unknown-elf

CFLAGS:= -mabi=ilp32 -march=rv32i -nostartfiles -ffreestanding -Os
CFLAGS+= -DENABLE_UART
CFLAGS+= -I $(RVATOM_LIB)/include
LFLAGS:= -L $(RVATOM_LIB)/ -T $(RVATOM_LIB)/link/link_bootloader.ld -lcatom -Xlinker -Map $(EXEC).map -Wl,--gc-sections

CONVELF_ARGS:= -t elf -c

ifeq ($(MAKECMDGOALS),clean)
else
    ifeq ($(soctarget), atombones)
        CFLAGS += -DTARGET_ATOMBONES -DTARGET_HEADER='"atombones.h"'
    else
    ifeq ($(soctarget), hydrogensoc)
        CFLAGS += -DTARGET_HYDROGENSOC -DTARGET_HEADER='"hydrogensoc.h"'
    else
       $(error Invalid SoC target: $(soctarget))
    endif
	endif
endif


default: $(EXEC).elf $(EXEC).objdump $(EXEC).bin $(EXEC).hex

$(EXEC).elf: main.c crt0.S
	$(RVPREFIX)-gcc $(CFLAGS) -o $@ $^ $(LFLAGS)

$(EXEC).objdump: $(EXEC).elf
	$(RVPREFIX)-objdump -htd $< > $@

$(EXEC).bin: $(EXEC).elf
	$(RVPREFIX)-objcopy -O binary $< $@

$(EXEC).hex: $(EXEC).elf
	convelf.py $(CONVELF_ARGS) -m="BOOTROM:0x00010000:8192:h:$@" $<

.PHONY: clean
clean:
	rm -f *.o *.objdump *.map *.elf *.hex *.bin