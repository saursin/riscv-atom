src_files = sdcard.c
executable = sdcard.elf
soctargets = hydrogensoc
include ../example.mk

# run attaches a blank 1 MiB SD card (writes stay in atomsim's memory)
ifneq ($(skip),1)
ATOMSIM_ARGS += --sdcard $(OUT)/sdcard.img
run: $(OUT)/sdcard.img
$(OUT)/sdcard.img:
	@mkdir -p $(OUT)
	$(RVATOM)/scripts/mkimg.py -o $@ --size 1M
endif
