soctarget ?= atombones

ifeq ($(soctarget),atombones)
COMPILE_SCR = compile-atombones
else
ifeq ($(soctarget),hydrogensoc)
COMPILE_SCR = compile-hydrogensoc
else
$(error Invalid soctarget)
endif
endif


RVPREFIX := riscv64-unknown-elf
ADDITIONAL_CFLAGS := -O3 -DTIME
ADDITIONAL_LFLAGS := -Xlinker -Map dhrystone.map


# CFLAGS = -mabi=ilp32 -march=rv32i -nostartfiles -ffreestanding -O3 
# CFLAGS += -I $(RVATOM_LIB)/include -L $(RVATOM_LIB) -T $(RVATOM_LIB)/link/link.ld -DTARGET_$(Target) -DTIME
# LFLAGS = -lcatom -Wl,--gc-sections -Xlinker -Map output.map 

SRC = dhry_1.c dhry_2.c util.c
HDR = dhry.h

# CFLAGS += $(DHRY-CFLAGS) $(XCFLAGS) -Xlinker --defsym=__stack_size=0x800 -Xlinker --defsym=__heap_size=0x1000

dhrystone: $(SRC) $(HDR)
	$(COMPILE_SCR) $(ADDITIONAL_CFLAGS) $(SRC) -o $@ $(ADDITIONAL_LFLAGS)
	$(RVPREFIX)-objdump -htd $@ > $@.objdump


clean:
	rm -f *.i *.s *.o dhrystone dhrystone.hex *.map *.objdump
