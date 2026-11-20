src_files = sdcard.c
executable = sdcard.elf
soctargets = hydrogensoc
include ../example.mk

# run attaches a blank 1 MiB SD card (writes stay in atomsim's memory)
ifneq ($(skip),1)
ATOMSIM_ARGS += --sdcard sdcard.img
run: sdcard.img
sdcard.img:
	$(RVATOM)/scripts/mkimg.py -o $@ --size 1M
endif

clean: clean-img
.PHONY: clean-img
clean-img:
	rm -f sdcard.img
