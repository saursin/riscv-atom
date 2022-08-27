#h *** ElfDump Builder ***

COLOR_CYAN := \033[36m
COLOR_NC   := \033[0m

BIN_DIR = bin
$(shell mkdir -p $(BIN_DIR))

CC = g++
CFLAGS = -Wall -I../../sim/include/elfio/
SRCS = elfdump.cpp
EXE = elfdump

#f Default TARGET: elfdump
default: elfdump						#t Alias for elfdump

.PHONY: elfdump
elfdump: $(BIN_DIR)/$(EXE)				#t Build elfdump

$(BIN_DIR)/$(EXE): $(SRCS)
	@printf "$(COLOR_CYAN)Compiling   %-25s <-  %s$(COLOR_NC)\n" $@ $<
	$(CC) $(CFLAGS) $^ -o $@
	@printf "$(COLOR_CYAN)Build successful!$(COLOR_NC)\n"


.PHONY: clean
clean:									#t clean build files
	rm -rf $(BIN_DIR)/*


.PHONY: help
help: 									#t show help
	@sed -n 's/^#h //p' Makefile
	@echo ""
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s#t\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t$(COLOR_CYAN)%-20s$(COLOR_NC) %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile