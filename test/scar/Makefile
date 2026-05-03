PY = python3

$(shell mkdir -p work)

default: clean verify

.PHONY: verify
verify:
	$(PY) -m pytest -v

clean:
	rm -rf work/*
