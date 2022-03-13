Target ?= hydrogensoc
SCRIPT = synth_xc6s.ys
REPORT = synth_report.txt
LOG = synth_synth.log

default: synth

.PHONY : help
help : Makefile		## show help
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	

.PHONY: synth
synth:		## synthesize
	@echo "Running Synthesis..."
	yosys -s $(SCRIPT) > $(LOG) 2>&1
	@echo "Generated log: 		$(LOG)"
	@echo "Generated report: 	$(REPORT)"


.PHONY: clean
clean:				## clean logs and reports
	rm -f *.log *.txt