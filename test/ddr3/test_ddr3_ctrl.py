"""cocotb tests for the vendored DDR3 controller on ddr3_phy_model (see tb_ddr3_ctrl.sv)."""
import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import FallingEdge

DQ_W, ROW_W, COL_W, BA_W, DW = 16, 13, 10, 3, 3
ADDR_W = BA_W + ROW_W + COL_W
LANES = DQ_W // 8
RD_SKEW = [1, 2]                    # per lane, as in tb_ddr3_ctrl.sv
MASK = (1 << DQ_W) - 1

# Signals are driven and sampled on falling clk edges, half a cycle away from the
# rising edges the RTL acts on.


async def start(dut):
    """Clock, reset, and wait for calibration."""
    cocotb.start_soon(Clock(dut.clk, 22, units="ns").start())
    dut.rst.value = 1
    dut.rd.value = dut.wr.value = dut.refresh.value = 0
    dut.addr.value = dut.din.value = dut.be.value = 0
    for _ in range(4):
        await FallingEdge(dut.clk)
    dut.rst.value = 0
    for _ in range(200_000):
        await FallingEdge(dut.clk)
        if dut.calib_done.value:
            break
    assert dut.calib_done.value, "calibration did not finish"
    assert not dut.calib_fail.value, "calibration failed"
    while dut.busy.value:
        await FallingEdge(dut.clk)


async def request(dut, **sig):
    """One request: a one-cycle pulse while busy is low; returns dout for reads."""
    while dut.busy.value:
        await FallingEdge(dut.clk)
    for name, val in sig.items():
        getattr(dut, name).value = val
    await FallingEdge(dut.clk)
    dut.rd.value = dut.wr.value = dut.refresh.value = 0
    is_read = sig.get("rd", 0)
    data = None
    for _ in range(1000):
        if is_read and dut.data_ready.value:
            data = (int(dut.dout.value), int(dut.dout_burst.value))
        if not dut.busy.value:
            break
        await FallingEdge(dut.clk)
    assert not dut.busy.value, f"request {sig} did not finish"
    if is_read:
        assert data is not None, "no data_ready for a read"
    return data


async def write(dut, addr, data, be=(1 << LANES) - 1):
    await request(dut, wr=1, addr=addr, din=data, be=be)


async def read(dut, addr):
    return (await request(dut, rd=1, addr=addr))[0]


@cocotb.test()
async def test_calibration(dut):
    """Calibration passes and picks the middle of each lane's two-step window."""
    await start(dut)
    delay = int(dut.rd_delay.value)
    got = [(delay >> (DW * l)) & ((1 << DW) - 1) for l in range(LANES)]
    # window = skew, skew+1; the middle of a 2-step window is its first step
    assert got == RD_SKEW, f"rd_delay {got}, expected {RD_SKEW}"


@cocotb.test()
async def test_write_read(dut):
    """Random words at random addresses (all banks, rows, columns) read back."""
    await start(dut)
    rng = random.Random(1)
    ref = {}
    for _ in range(200):
        addr = rng.getrandbits(ADDR_W)
        ref[addr] = rng.getrandbits(DQ_W)
        await write(dut, addr, ref[addr])
    for addr, val in rng.sample(list(ref.items()), len(ref)):
        got = await read(dut, addr)
        assert got == val, f"addr {addr:#x}: read {got:#06x}, wrote {val:#06x}"


@cocotb.test()
async def test_neighbours(dut):
    """A write touches only its own word: all 8 words of a burst stay independent."""
    await start(dut)
    base = 0x12345 << 3
    for j in range(8):
        await write(dut, base + j, 0x1111 * (j + 1))
    await write(dut, base + 3, 0xBEEF)
    for j in range(8):
        want = 0xBEEF if j == 3 else 0x1111 * (j + 1)
        assert await read(dut, base + j) == want, f"word {j}"


@cocotb.test()
async def test_byte_enables(dut):
    """Byte enables write only their byte lane."""
    await start(dut)
    addr = 0x0ABCDE
    await write(dut, addr, 0xAAAA)
    await write(dut, addr, 0x5555, be=0b01)
    assert await read(dut, addr) == 0xAA55
    await write(dut, addr, 0x3333, be=0b10)
    assert await read(dut, addr) == 0x3355
    await write(dut, addr, 0xFFFF, be=0b00)
    assert await read(dut, addr) == 0x3355


@cocotb.test()
async def test_burst(dut):
    """dout_burst returns the whole aligned 8-word burst, word 0 in the low bits."""
    await start(dut)
    base = 0x2F0F0 << 3
    words = [0xA000 + j for j in range(8)]
    for j, w in enumerate(words):
        await write(dut, base + j, w)
    dout, burst = await request(dut, rd=1, addr=base + 5)
    assert dout == words[5]
    assert [(burst >> (DQ_W * j)) & MASK for j in range(8)] == words


@cocotb.test()
async def test_refresh(dut):
    """Refreshes between accesses complete and keep the data."""
    await start(dut)
    rng = random.Random(2)
    ref = {}
    for i in range(50):
        addr = rng.getrandbits(ADDR_W)
        ref[addr] = rng.getrandbits(DQ_W)
        await write(dut, addr, ref[addr])
        if i % 5 == 0:
            await request(dut, refresh=1)
    for addr, val in ref.items():
        await request(dut, refresh=1)
        assert await read(dut, addr) == val
