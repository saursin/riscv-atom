.DEFAULT_GOAL := default

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
    ifneq ($(wildcard $(RVATOM)/rtl/config/$(soctarget).json),)
        ifneq ($(shell $(RVATOM)/scripts/cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -a type), soc)
            $(error Invalid soctarget=$(soctarget); config type not 'soc')
        endif
    else
        $(error Invalid soctarget=$(soctarget); config not found)
    endif
endif


##############################################################################
# common functions

# $1 level
# $2 msg
define print_info
	@printf "$(CLR_GR)>$(1)$(CLR_NC)\n"
endef

define print_warn
	@printf "$(CLR_YL)> WARNING:$(CLR_NC)$(1)\n"
endef

define print_error
	@printf "$(CLR_RD)> ERROR:$(CLR_NC)$(1)\n"
	$(error "Exiting...")
endef

##############################################################################
# common targets

.PHONY: help
help: Makefile
# Print Header comments '#h#'
	@sed -n 's/^#h# //p' $<

# Print Target descriptions '#t#'
	@printf "\nTARGETs:\n"
	@grep -E -h '\s#t#\s' $< | awk 'BEGIN {FS = ":.*?#t# "}; {printf "\t$(CLR_CY)%-20s$(CLR_NC) %s\n", $$1, $$2}'

# Print footer '#f#'
	@sed -n 's/^#f# //p' $<
