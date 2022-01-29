# Atomsim target (to be oerridden)
target = atombones

# current example (to be overridden)
ex = None

# Vuart for make run example
vuart = None

# all examples directories
EXAMPLE_DIRS = $(sort $(dir $(wildcard */)))

# include coresponding makefile.include file
ifeq ($(ex), None)
$(error Please specify an example application to build)
else
include $(ex)/Makefile.include
endif

# select compile script
ifeq ($(target), atombones)
COMPILE_SCRIPT = compile-atombones
else
ifeq ($(target), hydrogensoc)
COMPILE_SCRIPT = compile-hydrogensoc
else
$(error Invalid atomsim target: $(target))
endif
endif

# atomsim args
ATOMSIM_ARGS = --maxitr=999999999

# VUART setting
ifneq ($(vuart), None)
ATOMSIM_ARGS += --vuart=$(vuart)
endif

default: $(ex)/$(executable) # compile
all: compile_all run_all


SRC_FILES = $(patsubst %, $(ex)/, $(src_files))

# compile
$(ex)/$(executable): $(SRC_FILES)
	cd $(ex) && $(COMPILE_SCRIPT) $(src_files) -o $(executable)

# run
run: $(ex)/$(executable)
	atomsim $(ATOMSIM_ARGS) $^

# clean
clean:
	rm -f $(ex)/$(executable)
