#h# ***** RISC-V Atom examples *****
# Each example folder has its own Makefile (e.g. make -C banner run); this one builds, runs
# or cleans all of them. Variables such as soctarget= are passed on.

EXAMPLES := $(patsubst %/Makefile,%,$(wildcard */Makefile))

.PHONY: all
all:										#t# Build all examples
	@for ex in $(EXAMPLES); do $(MAKE) -s -C $$ex compile || exit 1; done

.PHONY: run-all
run-all:									#t# Build and run all examples
	@for ex in $(EXAMPLES); do $(MAKE) -s -C $$ex run || exit 1; done

.PHONY: clean-all
clean-all:									#t# Clean all examples
	@for ex in $(EXAMPLES); do $(MAKE) -s -C $$ex clean || exit 1; done

.PHONY: help
help:										#t# Show this help
	@python3 $(RVATOM)/scripts/genmkhelp.py Makefile
