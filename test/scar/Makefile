##########################
PY = python3

default: clean verify

verify: directories
	$(PY) scar.py

directories: work

work: 
	mkdir work

clean:
	rm -rf work/*

