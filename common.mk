.DEFAULT_GOAL := default

# RISC-V Toolchain
RV_PREFIX:= riscv64-unknown-elf-
CC:=$(RV_PREFIX)gcc
CXX:=$(RV_PREFIX)g++
OBJDUMP:=$(RV_PREFIX)objdump
GDB:=$(RV_PREFIX)gdb

# Config Parser
CFGPARSE:= $(RVATOM)/scripts/cfgparse.py

# Define bash color codes
ifndef NO_COLOR
CLR_RD := \033[0;31m
CLR_GR := \033[0;32m
CLR_YL := \033[0;33m
CLR_CY := \033[0;36m
CLR_BL := \033[34m
CLR_MG := \033[35m
CLR_B  := \033[1m
CLR_NB := \033[22m
CLR_D  := \033[2m
CLR_ND := \033[22m
CLR_NC := \033[0m
endif

##############################################################################
# Common Checks

# Check RVATOM environment variable
ifeq ($(_mk_check_env), 1)
    ifeq ($(RVATOM),)
        $(error RVATOM environment variable not set; did you forget to source the sourceme script?)
    endif
endif

# Default soctarget, persisted by 'make config' (gitignored). It uses '?=', so a
# soctarget from the command line or the environment takes precedence.
-include $(RVATOM)/config.mk

# Optional board: rtl/config/<soctarget>-<board>.yml imports the SoC config and
# overrides board facts (e.g. clk_freq); without a board the SoC config is used.
ifneq ($(strip $(soctarget)),)
    RVATOM_CFGNAME   := $(soctarget)$(if $(strip $(board)),-$(board))
    RVATOM_YMLCONFIG := $(RVATOM)/rtl/config/$(RVATOM_CFGNAME).yml
endif

# RVATOM_BUILD: root of all build artifacts, one tree per config by default.
RVATOM_BUILD ?= $(RVATOM)/build/$(RVATOM_CFGNAME)

# For targets that need it: require a valid soctarget and include the build
# parameters derived from its YAML (RVATOM_ISA, RVATOM_ABI, ...).
# Goals that are all 'help' skip the check; goals that are all clean goals need
# only the soctarget (to locate RVATOM_BUILD), not the derived config.
ifeq ($(_mk_check_soctarget), 1)
    _mk_goals_except = $(filter-out $(1),$(MAKECMDGOALS))
    _mk_help_only  := $(if $(MAKECMDGOALS),$(if $(call _mk_goals_except,help),,1),)
    _mk_clean_only := $(if $(MAKECMDGOALS),$(if $(call _mk_goals_except,clean clean-% %-clean help),,1),)
    ifneq ($(_mk_help_only), 1)
        ifeq ($(strip $(soctarget)),)
            $(error No soctarget set; run 'make config soctarget=<target>' or pass soctarget=<target>)
        endif
        ifeq ($(wildcard $(RVATOM_YMLCONFIG)),)
            $(error Invalid soctarget/board '$(RVATOM_CFGNAME)'; rtl/config/$(RVATOM_CFGNAME).yml not found)
        endif
    endif
    ifneq ($(_mk_clean_only), 1)
        # Named per config so a shared RVATOM_BUILD override never reuses
        # another config's values. Make regenerates it when any YAML it was
        # derived from (RVATOM_YMLDEPS) or cfgparse changes, then restarts.
        # The config headers are written with it: C/C++/asm builds force-include
        # atom_config.h (-include); RTL includes atom_config.vh explicitly.
        RVATOM_VARS_MK := $(RVATOM_BUILD)/vars.$(RVATOM_CFGNAME).mk
        RVATOM_CONFIG_H  := $(RVATOM_BUILD)/include/atom_config.h
        RVATOM_CONFIG_VH := $(RVATOM_BUILD)/include/atom_config.vh
        include $(RVATOM_VARS_MK)
        # RTL: verilator flags (include dirs + sources) and every file they read
        RVATOM_VFLAGS += +incdir+$(RVATOM_BUILD)/include
        RVATOM_VDEPS  += $(RVATOM_CONFIG_VH)
        $(RVATOM_VARS_MK) $(RVATOM_CONFIG_H) $(RVATOM_CONFIG_VH) &: $(RVATOM_YMLCONFIG) $(RVATOM_YMLDEPS) $(CFGPARSE)
		@mkdir -p $(RVATOM_BUILD)/include
		$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t header --style gcc -o $(RVATOM_CONFIG_H)
		$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t header --style verilator -o $(RVATOM_CONFIG_VH)
		$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t make-vars > $(RVATOM_VARS_MK).$$$$ && mv $(RVATOM_VARS_MK).$$$$ $(RVATOM_VARS_MK) || { rm -f $(RVATOM_VARS_MK).$$$$; exit 1; }
    endif
endif


##############################################################################
# common functions

# Print a formatted message 
define print_msg_root
	@printf "$(CLR_GR)$(CLR_B)❖ %-30b$(CLR_NB)%s$(CLR_NC)\n" "$(1)" "$(2)"
endef
define print_msg
	@printf "$(CLR_CY)$(CLR_B)► %-30b$(CLR_NB)%s$(CLR_NC)\n" "$(1)" "$(2)"
endef

# Print a formatted message with target information
define print_msgt_root
	@printf "$(CLR_GR)$(CLR_B)❖ %-30b$(CLR_NB)%s$(CLR_NC)\n" "$(1)" "$(@F)"
endef
define print_msgt
	@printf "$(CLR_CY)$(CLR_B)► %-30b$(CLR_NB)%s$(CLR_NC)\n" "$(1)" "$(@F)"
endef

define print_info
	@printf "$(CLR_CY)$(CLR_B)INFO:$(CLR_NB)$(CLR_NC)$(1)\n"
endef

define print_warn
	@printf "$(CLR_YL)$(CLR_B)WARN:$(CLR_NB)$(CLR_NC)$(1)\n"
endef

define print_error
	@printf "$(CLR_RD)$(CLR_B)ERROR:$(CLR_NB)$(CLR_NC)$(1)\n"
	@printf "Exiting..."; exit 1
endef

##############################################################################
# common targets

# the Makefile whose #h#/#v#/#t# tags 'make help' shows
HELP_MAKEFILE ?= Makefile

.PHONY: help
help: $(HELP_MAKEFILE)
	@python3 $(RVATOM)/scripts/genmkhelp.py $<
