import os
import subprocess
from pathlib import Path

import pytest

SCAR_DIR = Path(__file__).parent
WORK_DIR = SCAR_DIR / "work"


@pytest.fixture(scope="session")
def soctarget():
    result = subprocess.run(["atomsim", "--soctarget"], capture_output=True, text=True)
    if result.returncode != 0:
        pytest.skip("atomsim not available — build it first with `make sim`")
    return result.stdout.strip()


@pytest.fixture(scope="session")
def linker_script(soctarget):
    rvatom = os.environ.get("RVATOM")
    if not rvatom:
        pytest.skip("RVATOM not set — run `source sourceme` first")
    ld = Path(rvatom) / "sw" / "lib" / "link" / f"link_{soctarget}.ld"
    if not ld.exists():
        pytest.skip(f"Linker script not found: {ld}")
    return str(ld)


@pytest.fixture(scope="session", autouse=True)
def work_dir():
    WORK_DIR.mkdir(exist_ok=True)
    return WORK_DIR
