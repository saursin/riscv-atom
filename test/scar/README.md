# SCAR: instruction tests for Atom

A pytest suite: `test_instructions.py` runs every test in `tests.json` on atomsim and checks the
assertion block at the end of each `tests/*.S`. Full description and how to write tests:
[docs/pages/documentation/scar.rst](../../docs/pages/documentation/scar.rst).

```bash
make soctarget=hydrogensoc scar     # from the repo root: builds atomsim and runs the suite

cd test/scar                        # or directly with pytest
export ATOMSIM=$RVATOM/build/hydrogensoc/sim/bin/atomsim
pytest -v -k add
```
