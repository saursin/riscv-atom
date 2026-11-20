#h *** mkfatimg: FAT32 image from a directory (host tool, uses sw/lib/fatfs) ***

FATFS := ../../sw/lib/fatfs
BIN_DIR = bin
EXE = $(BIN_DIR)/mkfatimg

CC = gcc
CFLAGS = -O2 -Wall -I $(FATFS)

default: $(EXE)							#t Build mkfatimg

$(EXE): mkfatimg.c $(FATFS)/ff.c $(FATFS)/ffunicode.c $(FATFS)/ff.h $(FATFS)/ffconf.h $(FATFS)/diskio.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) mkfatimg.c $(FATFS)/ff.c $(FATFS)/ffunicode.c -o $@

.PHONY: clean
clean:									#t clean build files
	rm -rf $(BIN_DIR)
