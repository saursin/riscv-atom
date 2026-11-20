src_files = main.c
executable = fatfs.elf
soctargets = hydrogensoc
libs = fatfs
include ../example.mk

# run attaches an SD card holding the files under disk/ (a FAT32 image made by tools/mkfatimg)
MKFATIMG := $(RVATOM)/tools/mkfatimg/bin/mkfatimg
ifneq ($(skip),1)
ATOMSIM_ARGS += --sdcard $(OUT)/fatfs.img
run: $(OUT)/fatfs.img
$(OUT)/fatfs.img: $(MKFATIMG) $(shell find disk)
	@mkdir -p $(OUT)
	$(MKFATIMG) disk $@
$(MKFATIMG): $(RVATOM)/tools/mkfatimg/mkfatimg.c $(wildcard $(RVATOM_LIB)/fatfs/*)
	$(MAKE) -C $(RVATOM)/tools/mkfatimg
endif
