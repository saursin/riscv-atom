PY = python3

$(shell mkdir -p work)

default: clean verify

.PHONY: verify
verify:
	$(PY) scar.py -v tests.json

clean:
	rm -rf work/*
