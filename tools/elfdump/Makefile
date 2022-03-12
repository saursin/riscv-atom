CXX = g++
CXXFLAGS = -Wall -I../../sim/include/elfio/

SRCS = elfdump.cpp

BIN_DIR = bin
BIN = elfdump

default: $(BIN_DIR)/$(BIN)

$(BIN_DIR)/$(BIN): $(SRCS)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $^ -o $@


.PHONY: clean
clean:
	rm -rf $(BIN_DIR)