src_files = main.c
executable = fatfs.elf
soctargets = hydrogensoc
libs = fatfs
include ../example.mk

# run attaches an SD card holding the files under disk/ (a FAT32 image made by tools/mkfatimg)
MKFATIMG := $(RVATOM)/tools/mkfatimg/bin/mkfatimg
ifneq ($(skip),1)
ATOMSIM_ARGS += --sdcard fatfs.img
run: fatfs.img
fatfs.img: $(MKFATIMG) $(shell find disk)
	$(MKFATIMG) disk $@
$(MKFATIMG): $(RVATOM)/tools/mkfatimg/mkfatimg.c $(wildcard $(RVATOM_LIB)/fatfs/*)
	$(MAKE) -C $(RVATOM)/tools/mkfatimg
endif

clean: clean-img
.PHONY: clean-img
clean-img:
	rm -f fatfs.img
