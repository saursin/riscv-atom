"""cocotb tests for DDR3_wb on ddr3_phy_model (see tb_ddr3_wb.sv)."""
import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import FallingEdge

DQ_W, ROW_W, COL_W, BA_W, DW = 16, 13, 10, 3, 3
MEM_AW = BA_W + ROW_W + COL_W + 1                   # byte address bits (128 MB)
RD_SKEW = [1, 2]                                    # per lane, as in tb_ddr3_wb.sv
REFI = 45_000_000 // 1000 * 78 // 10000             # refresh period in clk cycles (351)
CMD_REF = 0b001

# Signals are driven and sampled on falling clk edges, half a cycle away from the
# rising edges the RTL acts on.


async def start(dut):
    """Clock and reset; does not wait for calibration."""
    cocotb.start_soon(Clock(dut.clk, 22, units="ns").start())
    dut.rst.value = 1
    dut.mem_stb.value = dut.ctrl_stb.value = 0
    dut.mem_adr.value = dut.mem_dat_i.value = dut.mem_we.value = dut.mem_sel.value = 0
    dut.ctrl_adr.value = 0
    for _ in range(4):
        await FallingEdge(dut.clk)
    dut.rst.value = 0


async def wb(dut, port, adr, we=0, dat=0, sel=0xF, timeout=200_000):
    """One classic Wishbone access; returns (read data, clk cycles from stb to ack)."""
    getattr(dut, port + "_adr").value = adr
    if port == "mem":
        dut.mem_we.value = we
        dut.mem_dat_i.value = dat
        dut.mem_sel.value = sel
    getattr(dut, port + "_stb").value = 1
    for cycles in range(1, timeout):
        await FallingEdge(dut.clk)
        if getattr(dut, port + "_ack").value:
            data = int(getattr(dut, port + "_dat_o").value)
            getattr(dut, port + "_stb").value = 0
            return data, cycles
    raise AssertionError(f"{port} access to {adr:#x} got no ack")


async def mem_write(dut, addr, data, sel=0xF):
    return (await wb(dut, "mem", addr >> 2, we=1, dat=data, sel=sel))[1]


async def mem_read(dut, addr):
    return await wb(dut, "mem", addr >> 2)


async def reg_read(dut, offset):
    return (await wb(dut, "ctrl", offset >> 2))[0]


async def ready(dut):
    await start(dut)
    for _ in range(200_000):
        await FallingEdge(dut.clk)
        if dut.ddr.c_calib_done.value:
            break
    assert await reg_read(dut, 0) == 0b01, "STATUS: expected READY, no CALIB_FAIL"


@cocotb.test()
async def test_status(dut):
    """Registers answer during calibration; READY afterwards; CALIB holds the delays."""
    await start(dut)
    assert await reg_read(dut, 0) == 0, "STATUS must not be READY during calibration"
    await ready(dut)
    calib = await reg_read(dut, 4)
    assert [(calib >> (DW * l)) & 7 for l in range(2)] == RD_SKEW, f"CALIB {calib:#x}"
    assert await reg_read(dut, 8) == 0 and await reg_read(dut, 12) == 0


@cocotb.test()
async def test_wait_for_calibration(dut):
    """A memory access right after reset waits until calibration is done."""
    await start(dut)
    _, cycles = await mem_read(dut, 0x100)
    assert dut.ddr.c_calib_done.value, "access completed before calibration"
    dut._log.info(f"first access completed after {cycles} cycles (calibration)")


@cocotb.test()
async def test_read_write(dut):
    """Random 32-bit words across the whole 128 MB read back."""
    await ready(dut)
    rng = random.Random(1)
    ref = {}
    for _ in range(200):
        addr = rng.getrandbits(MEM_AW) & ~3
        ref[addr] = rng.getrandbits(32)
        await mem_write(dut, addr, ref[addr])
    for addr, val in rng.sample(list(ref.items()), len(ref)):
        got, _ = await mem_read(dut, addr)
        assert got == val, f"{addr:#x}: read {got:#010x}, wrote {val:#010x}"


@cocotb.test()
async def test_byte_selects(dut):
    """Every sel pattern writes exactly its bytes; neighbouring words are untouched."""
    await ready(dut)
    base = 0x0123_4560
    mem = {base + 4 * i: 0 for i in range(-1, 17)}
    for addr in mem:
        await mem_write(dut, addr, 0)
    rng = random.Random(2)
    for sel in range(16):
        addr = base + 4 * sel
        data = rng.getrandbits(32)
        await mem_write(dut, addr, data, sel)
        bmask = sum(0xFF << (8 * b) for b in range(4) if sel >> b & 1)
        mem[addr] = (mem[addr] & ~bmask) | (data & bmask)
    for addr, want in mem.items():
        got, _ = await mem_read(dut, addr)
        assert got == want, f"{addr:#x}: read {got:#010x}, want {want:#010x}"


@cocotb.test()
async def test_refresh(dut):
    """The refresh timer issues one REFRESH per 7.8 us, while idle and between accesses.
    The model also fails the run if two refreshes are more than 2 x tREFI apart."""
    await ready(dut)
    refs = 0

    async def count():
        nonlocal refs
        while True:
            await FallingEdge(dut.clk)
            # the model takes commands on the clk edge where ce is low
            if not dut.ce.value and dut.phy_reset_n.value and int(dut.phy_cmd.value) == CMD_REF:
                refs += 1

    counter = cocotb.start_soon(count())
    idle = 20 * REFI
    for _ in range(idle):
        await FallingEdge(dut.clk)
    assert abs(refs - 20) <= 1, f"{refs} refreshes in 20 refresh periods while idle"

    refs = 0
    elapsed = 0
    rng = random.Random(3)
    ref = {}
    while elapsed < 20 * REFI:
        addr = rng.getrandbits(MEM_AW) & ~3
        ref[addr] = rng.getrandbits(32)
        elapsed += await mem_write(dut, addr, ref[addr])
    counter.kill()
    assert abs(refs - elapsed / REFI) <= 2, f"{refs} refreshes in {elapsed / REFI:.1f} periods of accesses"
    for addr, val in ref.items():
        assert (await mem_read(dut, addr))[0] == val


async def after_refresh(dut):
    """Wait until a refresh has just finished, so the next access isn't delayed by one."""
    while not (not dut.ce.value and dut.phy_reset_n.value and int(dut.phy_cmd.value) == CMD_REF):
        await FallingEdge(dut.clk)
    while dut.ddr.c_busy.value or int(dut.ddr.state.value) != 0:
        await FallingEdge(dut.clk)


@cocotb.test()
async def test_latency(dut):
    """Access latency in clk cycles, from stb to ack, each measured right after a refresh."""
    await ready(dut)
    addr = 0x0040_0000
    await after_refresh(dut)
    wr32 = await mem_write(dut, addr, 0x1234_5678)
    await after_refresh(dut)
    wr16 = await mem_write(dut, addr, 0xFFFF, sel=0b0011)
    await after_refresh(dut)
    wr8 = await mem_write(dut, addr, 0xFF, sel=0b0001)
    await after_refresh(dut)
    _, rd = await mem_read(dut, addr)
    dut._log.info(f"latency (clk cycles): read {rd}, write 32-bit {wr32}, 16-bit {wr16}, 8-bit {wr8}")
    assert wr16 == wr8 and wr32 > wr16, "a 32-bit write takes two controller writes, 8/16-bit one"
    assert rd < 60 and wr32 < 120, "unexpectedly slow"
