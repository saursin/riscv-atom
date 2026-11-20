#h# ***** Makefile for building FatFs (libfatfs.a) *****
# ChaN's FatFs (ff.c, ffunicode.c, ff.h, diskio.h: unmodified; ffconf.h: our settings) plus diskio_sd.c, the
# glue to libcatom's SD driver. Programs that use it link with -lfatfs -lcatom (in that order).
# Built only for soctargets with SPI.

_mk_check_env=1
_mk_check_soctarget=1
include ../../../common.mk
##################################################

OBJ_DIR := $(RVATOM_BUILD)/sw/lib/fatfs
LIB     := $(RVATOM_BUILD)/sw/lib/libfatfs.a

RVPREFIX := riscv64-unknown-elf
CFLAGS   := -march=$(RVATOM_ISA) -mabi=$(RVATOM_ABI)
CFLAGS   += -Wall -ffreestanding -nostartfiles -nostdlib -I ../include -Os
CFLAGS   += -include $(RVATOM_CONFIG_H) -MMD -MP
CFLAGS   += -fdata-sections -ffunction-sections	# programs drop what they don't use (--gc-sections)

SRCS := ff.c ffunicode.c diskio_sd.c
OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

HAS_SPI := $(shell grep -q 'define SOC_EN_SPI' $(RVATOM_CONFIG_H) && echo 1)

default: build
ifeq ($(HAS_SPI),1)
build: $(LIB)							#t# Build libfatfs.a
else
build:
	@printf "$(CLR_YL)  skipped:$(CLR_NC) FatFs needs SPI ($(soctarget) has none)\n"
endif

$(LIB): $(OBJS)
	$(call print_msgt,Building)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: %.c $(RVATOM_VARS_MK)
	@mkdir -p $(OBJ_DIR)
	$(call print_msgt,Compiling)
	$(RVPREFIX)-gcc -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:									#t# Clean build files
	rm -rf $(OBJ_DIR) $(LIB)

-include $(OBJS:.o=.d)
