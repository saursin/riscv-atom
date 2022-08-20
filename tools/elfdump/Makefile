#h *** ElfDump Builder ***

BIN_DIR = bin
$(shell mkdir -p $(BIN_DIR))

CC = g++
CFLAGS = -Wall -I../../sim/include/elfio/
SRCS = elfdump.cpp
EXE = elfdump

#f Default TARGET: elfdump
default: elfdump

.PHONY: elfdump
elfdump: $(BIN_DIR)/$(EXE)				#t build elfdump

$(BIN_DIR)/$(EXE): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@


.PHONY: clean
clean:									#t clean build files
	rm -rf $(BIN_DIR)/*


.PHONY: super-clean
super-clean:							#t Revert to untouched state
	rm -rf $(BIN_DIR)/


.PHONY: help
help: 									#t show help
	@sed -n 's/^#h //p' Makefile
	@echo ""
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s#t\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile