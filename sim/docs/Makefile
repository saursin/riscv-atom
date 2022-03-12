DOXYFILE = Doxyfile


default: html-latex

.PHONY: html-latex
html-latex: $(doc_dir) $(doxygen_doc_dir)		## generate doxygen docs (html & latex)
	doxygen $(DOXYFILE)

.PHONY: pdf
pdf: html-latex									## generate doxygen docs (pdf)
	make -C build/latex/
	mv build/latex/refman.pdf build/AtomSim.pdf


.PHONY: help
help: 									## show help
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'


.PHONY: clean
clean:									## clean build files
	rm -rf build/
