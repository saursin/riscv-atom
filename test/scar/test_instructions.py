import json
import re
import subprocess
from pathlib import Path

import pytest

from conftest import SCAR_DIR, WORK_DIR

# ---------------------------------------------------------------------------
# Register alias map (ABI names → canonical xN names)
# ---------------------------------------------------------------------------
_REG_ALIASES = {
    "zero": "x0",  "ra": "x1",  "sp": "x2",  "gp": "x3",  "tp": "x4",
    "t0":   "x5",  "t1": "x6",  "t2": "x7",
    "s0":   "x8",  "fp": "x8",  "s1": "x9",
    "a0":   "x10", "a1": "x11", "a2": "x12", "a3": "x13",
    "a4":   "x14", "a5": "x15", "a6": "x16", "a7": "x17",
    "s2":   "x18", "s3": "x19", "s4": "x20", "s5": "x21",
    "s6":   "x22", "s7": "x23", "s8": "x24", "s9": "x25",
    "s10":  "x26", "s11":"x27",
    "t3":   "x28", "t4": "x29", "t5": "x30", "t6": "x31",
}


def _parse_dump(dump_file: Path) -> dict:
    """Parse 'name : hexvalue' lines from an atomsim dump file."""
    regs = {}
    for line in dump_file.read_text().splitlines():
        if ":" not in line:
            continue
        name, _, val = line.partition(":")
        name, val = name.strip(), val.strip()
        if name and val:
            try:
                regs[name] = int(val, 16)
            except ValueError:
                pass
    # add ABI aliases so assertions can use either 'a0' or 'x10'
    for alias, canon in _REG_ALIASES.items():
        if canon in regs:
            regs[alias] = regs[canon]
    return regs


def _extract_assertions(src_file: Path) -> list:
    """Return assertion expressions from the #>>>ASSERT ... #<<<END_ASSERT block."""
    assertions = []
    in_block = False
    for line in src_file.read_text().splitlines():
        stripped = line.strip()
        if stripped == "#>>>ASSERT":
            in_block = True
            continue
        if stripped == "#<<<END_ASSERT":
            break
        if in_block and stripped.startswith("#"):
            expr = stripped[1:].strip()          # strip leading '#'
            expr = expr.split("#", 1)[0].strip() # strip inline comment
            if expr:
                assertions.append(expr)
    return assertions


# ---------------------------------------------------------------------------
# Test parametrization
# ---------------------------------------------------------------------------
with open(SCAR_DIR / "tests.json") as _f:
    _TESTS = json.load(_f)


@pytest.mark.parametrize("test_case", _TESTS, ids=lambda t: t["name"])
def test_instruction(test_case, soctarget, linker_script, work_dir):
    name = test_case["name"]
    src_files = [SCAR_DIR / s for s in test_case["srcs"]]
    elf_file  = work_dir / f"{name}.elf"
    dump_file = work_dir / f"{name}.dump"

    # ---- compile ----
    result = subprocess.run(
        [
            "riscv64-unknown-elf-gcc",
            "-march=rv32im", "-mabi=ilp32", "-nostartfiles",
            *[str(f) for f in src_files],
            "-o", str(elf_file),
            "-T", linker_script,
        ],
        capture_output=True, text=True,
    )
    assert result.returncode == 0, f"Compile failed:\n{result.stderr}"

    # ---- simulate ----
    # Note: atomsim does freopen("/dev/tty") to restore stdout after writing the
    # dump, so "Exiting.." prints directly to the terminal — that is expected.
    dump_file.unlink(missing_ok=True)  # ensure stale dump is not reused
    result = subprocess.run(
        [
            "atomsim",
            "--ebreak-dump", "--no-banner",
            "--maxitr", "100000",
            "--dump-file", str(dump_file),
            str(elf_file),
        ],
        capture_output=True, text=True,
    )
    assert result.returncode == 0, f"Simulation failed:\n{result.stderr or result.stdout}"
    assert "EBreak hit" in result.stdout, \
        f"ebreak was never reached (maxitr exceeded or early exit):\n{result.stdout}"
    assert dump_file.exists() and dump_file.stat().st_size > 0, \
        "Dump file was not written"

    # ---- check assertions ----
    regs = _parse_dump(dump_file)
    assertions = _extract_assertions(src_files[0])
    assert assertions, "No assertions found — add a #>>>ASSERT / #<<<END_ASSERT block to the .S file"

    failures = []
    for expr in assertions:
        try:
            passed = eval(expr, {"__builtins__": {}}, regs)  # noqa: S307
        except Exception as exc:
            failures.append(f"  ERROR evaluating {expr!r}: {exc}")
            continue
        if not passed:
            # resolve register names for a useful error message
            annotated = re.sub(
                r"\b([a-z][a-z0-9]*)\b",
                lambda m: f"{m.group(0)}({hex(regs[m.group(0)])})" if m.group(0) in regs else m.group(0),
                expr,
            )
            failures.append(f"  FAIL: {annotated}")

    assert not failures, "Assertion failures:\n" + "\n".join(failures)
