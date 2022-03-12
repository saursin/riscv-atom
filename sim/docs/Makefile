#h *** AtomSim Code Documentation Generator ***

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


#f Default TARGET: doc
default: doc
all: doc pdf


.PHONY: doc
doc: $(HTML_DIR)/index.html				#t generate doxygen docs (html & latex)

$(HTML_DIR)/index.html: $(SRC_FILES)
	doxygen $(DOXYFILE)


.PHONY: pdf
pdf: $(MAN_PDF)							#t generate doxygen docs (pdf)

$(MAN_PDF): $(LATEX_DIR)/refman.pdf
	mv $^ $@

$(LATEX_DIR)/refman.pdf:
	make -C build/latex/



.PHONY: help
help: 									#t show help
	@sed -n 's/^#h //p' Makefile
	@echo ""
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s#t\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?#t "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@sed -n 's/^#f //p' Makefile


.PHONY: clean
clean:									#t clean build files
	rm -rf $(HTML_DIR)/*
	rm -rf $(LATEX_DIR)/*

.PHONY: super-clean
super-clean:							#t clean build dir
	rm -rf build/
