// DDR3 memory test (HydrogenSoC with peripherals.ddr, e.g. the TangPrimer 20K).
// Waits for calibration, then tests the data bus (walking ones), the address bus (one word
// per address bit, so aliasing shows), byte and halfword writes, and an address-in-address
// pattern over BLOCK bytes, and measures sequential write and read speed.
#include <stdio.h>
#include <time.h>
#include <platform.h>
#include <mmio.h>

#define BLOCK           (32 * 1024)     // bytes for the memtest and memspeed passes

#define DDR_STATUS      REG32(SOC_DDR_CTRL_ADDR, 0x0)
#define DDR_CALIB       REG32(SOC_DDR_CTRL_ADDR, 0x4)
#define STATUS_READY    (1 << 0)
#define STATUS_FAIL     (1 << 1)

static volatile uint32_t *const ddr = (volatile uint32_t *)SOC_DDR_ADDR;
static unsigned errors;

static void check(uint32_t off, uint32_t got, uint32_t want)
{
    if(got == want)
        return;
    if(errors++ < 8)
        printf("  error @0x%08x: read 0x%08x, want 0x%08x\n", SOC_DDR_ADDR + off, got, want);
}

// prints OK or FAIL for the errors since `before`
static void result(unsigned before)
{
    if(errors == before)
        puts("OK\n");
    else
        printf("FAIL (%u errors)\n", errors - before);
}

// bytes per second -> "x.y MiB/s" from a cycle count
static void print_speed(const char *what, uint32_t bytes, clock_t cycles)
{
    uint32_t clk = platform_clk();
    uint32_t tenths = (uint32_t)((unsigned long long)bytes * clk * 10 / cycles / (1024 * 1024));
    printf("  %s speed: %u.%u MiB/s (%u cycles per word)\n", what, tenths / 10, tenths % 10,
           (unsigned)(cycles / (bytes / 4)));
}

void main()
{
    serial_init(UART_BAUD_115200);

    uint32_t size = platform_ddr_size();
    if(!size) {
        puts("ddrtest: this board has no DDR\n");
        return;
    }
    printf("DDR3 @0x%08x (%u MiB), registers @0x%08x\n", SOC_DDR_ADDR, size >> 20, SOC_DDR_CTRL_ADDR);

    // calibration finishes about 0.7 ms after reset
    puts("Waiting for initialization and read calibration... ");
    uint32_t st;
    clock_t t0 = cycle();
    while(!((st = DDR_STATUS) & (STATUS_READY | STATUS_FAIL)))
        ;
    clock_t waited = cycle() - t0;
    uint32_t calib = DDR_CALIB;
    if(st & STATUS_FAIL) {
        puts("FAIL\n");
        printf("  read delays: lane 0: %u, lane 1: %u (no passing window)\n", calib & 7, (calib >> 3) & 7);
        return;
    }
    printf("done (waited %u us)\n", (unsigned)(waited / (platform_clk() / 1000000)));
    printf("  read delays: lane 0: %u, lane 1: %u (half-cycle steps, chosen by hardware)\n",
           calib & 7, (calib >> 3) & 7);

    unsigned e;

    // data bus: walking ones
    puts("Data bus test (walking ones)... ");
    e = errors;
    for(int b = 0; b < 32; b++) {
        ddr[0] = 1u << b;
        check(0, ddr[0], 1u << b);
    }
    result(e);

    // address bus: a distinct word at every power-of-two offset, then check none aliased
    uint32_t nbits = 0;
    while((4u << nbits) < size)
        nbits++;
    printf("Address bus test (%u address bits)... ", nbits + 2);
    e = errors;
    for(uint32_t k = 0; k < nbits; k++)
        ddr[1u << k] = 0xA5000000 | k;
    ddr[0] = 0x5A5A5A5A;
    check(0, ddr[0], 0x5A5A5A5A);
    for(uint32_t k = 0; k < nbits; k++)
        check(4u << k, ddr[1u << k], 0xA5000000 | k);
    result(e);

    // byte and halfword writes
    puts("Byte and halfword writes... ");
    e = errors;
    volatile uint8_t *b8 = (volatile uint8_t *)ddr;
    volatile uint16_t *b16 = (volatile uint16_t *)ddr;
    ddr[1] = 0;
    b8[4] = 0x11; b8[5] = 0x22; b8[6] = 0x33; b8[7] = 0x44;
    check(4, ddr[1], 0x44332211);
    b16[3] = 0xBEEF;
    check(4, ddr[1], 0xBEEF2211);
    check(5, b8[5], 0x22);
    result(e);

    // memtest: address-in-address (inverted), timed for the speed figures
    uint32_t n = BLOCK / 4;
    printf("Memtest at 0x%08x (%u KiB)...\n", SOC_DDR_ADDR, BLOCK >> 10);
    e = errors;
    printf("  Write: 0x%08x-0x%08x\n", SOC_DDR_ADDR, SOC_DDR_ADDR + BLOCK);
    t0 = cycle();
    for(uint32_t i = 0; i < n; i++)
        ddr[i] = ~(SOC_DDR_ADDR + 4 * i);
    clock_t t_wr = cycle() - t0;
    printf("   Read: 0x%08x-0x%08x\n", SOC_DDR_ADDR, SOC_DDR_ADDR + BLOCK);
    t0 = cycle();
    for(uint32_t i = 0; i < n; i++)
        check(4 * i, ddr[i], ~(SOC_DDR_ADDR + 4 * i));
    clock_t t_rd = cycle() - t0;
    puts(errors == e ? "Memtest OK\n" : "Memtest FAIL\n");

    printf("Memspeed at 0x%08x (sequential, %u KiB, %u MHz)...\n", SOC_DDR_ADDR, BLOCK >> 10,
           platform_clk() / 1000000);
    print_speed("Write", BLOCK, t_wr);
    print_speed(" Read", BLOCK, t_rd);       // includes the compare

    if(errors)
        printf("FAIL: %u errors\n", errors);
    else
        puts("PASS\n");
}
