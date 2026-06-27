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

# Check if soctarget is correct
ifeq ($(_mk_check_soctarget), 1)
    ifeq ($(wildcard $(RVATOM)/rtl/config/$(soctarget).yml),)
        $(error Invalid soctarget=$(soctarget); config not found)
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

.PHONY: help
help: Makefile
	@python3 $(RVATOM)/scripts/genmkhelp.py $<
