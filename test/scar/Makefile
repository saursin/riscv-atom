PY = python3

$(shell mkdir -p work)

default: clean verify

.PHONY: verify
verify:
	$(PY) scar.py

clean:
	rm -rf work/*
