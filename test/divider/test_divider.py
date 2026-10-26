"""cocotb tests for rtl/lib/Divider.v: results and cycle counts."""
import os
import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, ReadOnly

WIDTH = int(os.environ.get("WIDTH", 32))
SKIP_W = int(os.environ.get("SKIP_W", 8))
MASK = (1 << WIDTH) - 1
SMIN = 1 << (WIDTH - 1)


def to_signed(x):
    return x - (1 << WIDTH) if x & SMIN else x


def reference(a, b, signed):
    """RISC-V DIV/DIVU/REM/REMU semantics; returns (quotient, remainder, cycles)."""
    if b == 0:
        return MASK, a, 1
    if signed:
        sa, sb = to_signed(a), to_signed(b)
        q = abs(sa) // abs(sb)
        if (sa < 0) != (sb < 0):
            q = -q
        r = sa - q * sb
        mag = abs(sa)
    else:
        q, r = a // b, a % b
        mag = a
    # leading all-zero SKIP_W-bit chunks of |dividend| are skipped
    skip = 0
    for c in reversed(range(WIDTH // SKIP_W)):
        if (mag >> (c * SKIP_W)) & ((1 << SKIP_W) - 1):
            break
        skip += 1
    return q & MASK, r & MASK, 2 + WIDTH - skip * SKIP_W


async def reset(dut):
    dut.start_i.value = 0
    dut.signed_i.value = 0
    dut.dividend_i.value = 0
    dut.divisor_i.value = 0
    dut.rst_i.value = 1
    for _ in range(2):
        await RisingEdge(dut.clk_i)
    dut.rst_i.value = 0
    await RisingEdge(dut.clk_i)


async def divide(dut, a, b, signed):
    """Issue one division; return (quotient, remainder, cycles start->done)."""
    dut.dividend_i.value = a
    dut.divisor_i.value = b
    dut.signed_i.value = int(signed)
    dut.start_i.value = 1
    await RisingEdge(dut.clk_i)
    dut.start_i.value = 0
    cycles = 0
    while True:
        await ReadOnly()
        cycles += 1
        if dut.done_o.value:
            q, r = int(dut.quotient_o.value), int(dut.remainder_o.value)
            await RisingEdge(dut.clk_i)
            return q, r, cycles
        await RisingEdge(dut.clk_i)


async def check(dut, a, b, signed):
    got = await divide(dut, a, b, signed)
    exp = reference(a, b, signed)
    assert got == exp, (
        f"{'s' if signed else 'u'} {a:#x} / {b:#x}: got q={got[0]:#x} r={got[1]:#x} "
        f"cyc={got[2]}, expected q={exp[0]:#x} r={exp[1]:#x} cyc={exp[2]}")


async def setup(dut):
    cocotb.start_soon(Clock(dut.clk_i, 10, units="ns").start())
    await reset(dut)


def edge_operands():
    vals = {0, 1, 2, 3, MASK, MASK - 1, SMIN, SMIN - 1, SMIN + 1}
    for j in range(1, WIDTH // SKIP_W + 1):       # around every skip boundary
        b = 1 << (j * SKIP_W)
        vals |= {(b - 1) & MASK, b & MASK, (b + 1) & MASK, (b >> 1) & MASK}
    return sorted(vals)


@cocotb.test()
async def test_edge_cases(dut):
    """Divide-by-zero, MIN/-1, +-1 and skip-boundary operands, both signednesses."""
    await setup(dut)
    ops = edge_operands()
    for signed in (False, True):
        for a in ops:
            for b in ops:
                await check(dut, a, b, signed)


@cocotb.test(skip=WIDTH > 8)
async def test_exhaustive(dut):
    """Every operand pair (small WIDTH only)."""
    await setup(dut)
    for signed in (False, True):
        for a in range(1 << WIDTH):
            for b in range(1 << WIDTH):
                await check(dut, a, b, signed)


@cocotb.test()
async def test_random(dut):
    """Random operands, biased towards small magnitudes to hit the skip paths."""
    await setup(dut)
    rng = random.Random(int(os.environ.get("SEED", 1)))
    for _ in range(3000):
        bits_a = rng.randint(1, WIDTH)
        bits_b = rng.randint(1, WIDTH)
        a = rng.getrandbits(bits_a)
        b = rng.getrandbits(bits_b)
        if rng.random() < 0.5:                      # negative operands too
            a = (-a) & MASK
        if rng.random() < 0.5:
            b = (-b) & MASK
        await check(dut, a, b, rng.random() < 0.5)


@cocotb.test()
async def test_start_ignored_while_busy(dut):
    """A start pulse during a division neither restarts nor corrupts it."""
    await setup(dut)
    a, b = MASK, 3
    dut.dividend_i.value = a
    dut.divisor_i.value = b
    dut.signed_i.value = 0
    dut.start_i.value = 1
    await RisingEdge(dut.clk_i)
    dut.start_i.value = 0
    await RisingEdge(dut.clk_i)
    # glitch a new request mid-division
    dut.dividend_i.value = 7
    dut.divisor_i.value = 0
    dut.start_i.value = 1
    await RisingEdge(dut.clk_i)
    dut.start_i.value = 0
    while True:
        await ReadOnly()
        if dut.done_o.value:
            break
        await RisingEdge(dut.clk_i)
    q, r, _ = reference(a, b, False)
    assert (int(dut.quotient_o.value), int(dut.remainder_o.value)) == (q, r)
