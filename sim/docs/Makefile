DOXYFILE = Doxyfile

SRC_DIR = ..
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(SRC_DIR)/*.hpp)

BUILD_DIR = build
LATEX_DIR = $(BUILD_DIR)/latex
HTML_DIR = $(BUILD_DIR)/html

# create dirs at parse time
$(shell mkdir -p $(HTML_DIR) $(LATEX_DIR))

MAN_PDF = $(BUILD_DIR)/AtomSim.pdf


default: html-latex
all: html-latex pdf


.PHONY: html-latex
html-latex: $(HTML_DIR)/index.html		## generate doxygen docs (html & latex)

$(HTML_DIR)/index.html: $(SRC_FILES)
	doxygen $(DOXYFILE)


.PHONY: pdf
pdf: $(MAN_PDF)							## generate doxygen docs (pdf)

$(MAN_PDF): $(LATEX_DIR)/refman.pdf
	mv $^ $@

$(LATEX_DIR)/refman.pdf:
	make -C build/latex/
	


.PHONY: help
help: 									## show help
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'


.PHONY: clean
clean:									## clean build files
	rm -rf build/*

.PHONY: super-clean
super-clean:							## clean build dir
	rm -rf build/