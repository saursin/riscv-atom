include ../../common.mk

TIMESTAMP=$(shell date +"%m-%d-%Y_%H-%M-%S")
COMMITHASH=$(shell git rev-parse --short HEAD)
REPORT_FILE := synth_$(COMMITHASH)_$(TIMESTAMP).rpt
LOG_FILE := synth_$(COMMITHASH)_$(TIMESTAMP).log


TCL_SCRIPT ?= synth_xilinx.tcl
TCL_ARGS := ../../rtl/soc/hydrogensoc.F $(REPORT_FILE)



default: synth

.PHONY : help
help : Makefile		## show help
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	

.PHONY: synth
synth:				## synthesize
	@echo ">> Getting rom hex image.."
	make -C $(RVATOM)/sw/bootloader soctarget=hydrogensoc
	cp $(RVATOM)/sw/bootloader/bootloader.hex rom.hex

	@echo ">> Running Synthesis.."
	yosys -p 'tcl $(TCL_SCRIPT) $(TCL_ARGS)' 2>&1 > $(LOG_FILE)

	@echo "Log: 	$(LOG_FILE)"
	@echo "Report: 	$(REPORT_FILE)"


.PHONY: clean
clean:				## clean logs and reports
	rm -f *.log *.rpt *.hex