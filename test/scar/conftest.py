import os
import shutil
import subprocess
from pathlib import Path

import pytest

SCAR_DIR = Path(__file__).parent
WORK_DIR = SCAR_DIR / "work"


@pytest.fixture(scope="session")
def atomsim():
    # `make scar` sets ATOMSIM to the soctarget's build; else fall back to PATH
    return os.environ.get("ATOMSIM", "atomsim")


@pytest.fixture(scope="session")
def soctarget(atomsim):
    try:
        result = subprocess.run([atomsim, "--soctarget"], capture_output=True, text=True)
    except FileNotFoundError:
        result = None
    if result is None or result.returncode != 0:
        pytest.skip(f"atomsim not found ({atomsim}) — build it with `make sim` and set ATOMSIM")
    return result.stdout.strip()


@pytest.fixture(scope="session")
def linker_script(atomsim, soctarget):
    # preprocessed by the libcatom build: <build>/sw/lib/, next to <build>/sim/bin/atomsim
    build = Path(shutil.which(atomsim)).resolve().parents[2]
    ld = build / "sw" / "lib" / f"link_{soctarget}.ld"
    if not ld.exists():
        pytest.skip(f"Linker script not found: {ld} — build libcatom (`make lib`)")
    return str(ld)


@pytest.fixture(scope="session", autouse=True)
def work_dir():
    WORK_DIR.mkdir(exist_ok=True)
    return WORK_DIR
