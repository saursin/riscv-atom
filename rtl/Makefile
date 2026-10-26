#h# ***** RTL Build Makefile *****

#v# Stage mode: symlink (default) or copy
MODE ?= symlink

_mk_check_env := 1
_mk_check_soctarget := 1
include ../common.mk

BUILD_DIR := $(RVATOM_BUILD)/rtl

VTOPMODULE := $(RVATOM_TOPMODULE)

VC     := verilator
VFLAGS := -Wall --top-module $(VTOPMODULE)

$(shell mkdir -p $(BUILD_DIR))

####################################################

default: build

.PHONY: build
build: $(BUILD_DIR)/filelist.f					#t# Generate filelist and stage RTL files in build dir

$(BUILD_DIR)/filelist.f: $(RVATOM_VARS_MK)
	@mkdir -p $(BUILD_DIR)
	$(call print_msg,Generating filelist,$(BUILD_DIR)/filelist.f)
	@$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t filelist --base-dir $(abspath $(BUILD_DIR)) -o $@
	$(call print_msg,Staging RTL files (MODE=$(MODE)))
	@for f in $$($(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t vsrcs); do \
		if [ "$(MODE)" = "copy" ]; then \
			cp -f $$f $(BUILD_DIR)/; \
		else \
			ln -sf $$f $(BUILD_DIR)/; \
		fi \
	done

.PHONY: lint
lint: $(BUILD_DIR)/filelist.f					#t# Run verilator lint-only on RTL
	$(call print_msg,Running lint)
	@if $(VC) $(VFLAGS) --lint-only -f $<; then \
		printf "$(CLR_CY)$(CLR_B)► %-30b$(CLR_NB)%s$(CLR_NC)\n" "Lint check" "OK"; \
	else \
		printf "$(CLR_CY)$(CLR_B)► %-30b$(CLR_NB)%s$(CLR_NC)\n" "Lint check" "Errors Found"; \
		exit 1; \
	fi

.PHONY: clean
clean:											#t# Remove RTL build directory
	$(call print_msg,Cleaning RTL build files)
	rm -rf $(BUILD_DIR)
