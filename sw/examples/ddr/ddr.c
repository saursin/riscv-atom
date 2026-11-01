// DDR3 example (HydrogenSoC with peripherals.ddr, e.g. the TangPrimer 20K), in two parts:
// - Image in DDR: if a grayscale BMP (8 bits per pixel, uncompressed) was loaded at DDR_IN, its
//   Sobel edges (min(|gx| + |gy|, 255), border pixels 0) are computed into a DDR_NOINIT buffer
//   and sent back as edges.bmp, with the input's header and palette, once all of it is done.
//     atomterm <port> --load 0x80080000:cameraman.bmp --load ddr.bin --run
//   cameraman.bmp (256x256) is a test image; make others with, e.g.,
//     convert in.png -colorspace gray -depth 8 BMP3:image.bmp
// - Memory tests (first): calibration, data bus (walking ones), address bus (one word per address
//   bit, so aliasing shows), byte and halfword writes, an address-in-address pattern over BLOCK
//   bytes, and sequential write and read speed. They run on DDR above IMAGE_AREA, which holds
//   the image test's input and output.
#include <stdio.h>
#include <time.h>
#include <platform.h>
#include <mmio.h>
#include <xmodem.h>

#define BLOCK           (32 * 1024)     // bytes for the memtest and memspeed passes

#define DDR_STATUS      REG32(SOC_DDR_CTRL_ADDR, 0x0)
#define DDR_CALIB       REG32(SOC_DDR_CTRL_ADDR, 0x4)
#define STATUS_READY    (1 << 0)
#define STATUS_FAIL     (1 << 1)

#define MAX_FILE        (512 * 1024)
#define DDR_IN          (SOC_DDR_ADDR + MAX_FILE)   // the input BMP, after the output
#define IMAGE_AREA      (2 * MAX_FILE)              // output + input (1 MiB); the memory tests use the rest

static uint8_t out[MAX_FILE] DDR_NOINIT;            // the output BMP (at the start of DDR)
static unsigned errors;                             // memory test mismatches
static uint32_t mem_base;                           // start of the memory tests' region

// little-endian fields, read byte by byte (BMP fields are not aligned)
static uint32_t rd16(const volatile uint8_t *p) { return p[0] | p[1] << 8; }
static uint32_t rd32(const volatile uint8_t *p) { return rd16(p) | rd16(p + 2) << 16; }

static void check(uint32_t off, uint32_t got, uint32_t want)
{
    if(got == want)
        return;
    if(errors++ < 8)
        printf("  error @0x%08x: read 0x%08x, want 0x%08x\n", mem_base + off, got, want);
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

// Sobel edges of the BMP at DDR_IN, sent back as edges.bmp; skipped without one.
// Returns the number of failures (0 or 1).
static unsigned test_image()
{
    const volatile uint8_t *bmp = (const volatile uint8_t *)DDR_IN;
    puts("Image in DDR: ");
    if(bmp[0] != 'B' || bmp[1] != 'M') {
        printf("no BMP at 0x%08x, skipped\n", DDR_IN);
        return 0;
    }
    uint32_t off = rd32(bmp + 10), w = rd32(bmp + 18);
    int32_t hs = rd32(bmp + 22);                     // negative: rows top-down
    uint32_t h = hs < 0 ? -hs : hs;
    uint32_t stride = (w + 3) & ~3u;                 // rows are padded to 4 bytes
    if(rd16(bmp + 28) != 8 || rd32(bmp + 30) != 0 || w < 3 || h < 3 || off + stride * h > MAX_FILE) {
        puts("not an 8-bit uncompressed BMP of a supported size\n");
        return 1;
    }
    const volatile uint8_t *in = bmp + off;
    printf("%ux%u BMP at 0x%08x, computing Sobel edges\n", w, h, DDR_IN);
    for(uint32_t i = 0; i < off; i++)                // header and palette, as in the input
        out[i] = bmp[i];
    clock_t t0 = cycle();
    int s = stride;                                  // row to row
    for(uint32_t y = 0; y < h; y++)
        for(uint32_t x = 0; x < stride; x++) {
            int e = 0;
            if(x > 0 && y > 0 && x < w - 1 && y < h - 1) {
                const volatile uint8_t *p = in + y * stride + x;    // centre; rows either side
                int gx = (p[-s + 1] + 2 * p[1] + p[s + 1]) - (p[-s - 1] + 2 * p[-1] + p[s - 1]);
                int gy = (p[s - 1] + 2 * p[s] + p[s + 1]) - (p[-s - 1] + 2 * p[-s] + p[-s + 1]);
                e = (gx < 0 ? -gx : gx) + (gy < 0 ? -gy : gy);
                if(e > 255)
                    e = 255;
            }
            out[off + y * stride + x] = e;
        }
    printf("  done in %u cycles; sending edges.bmp\n", (unsigned)(cycle() - t0));
    int r = xmodem_send("edges.bmp", out, off + stride * h);
    if(r) {
        printf("\n  send failed (%d)\n", r);
        return 1;
    }
    puts("\n  sent\n");
    return 0;
}

// Data bus, address bus, byte and halfword writes, memtest and speed, writing only within
// [base, base + size). Returns the number of mismatches.
static unsigned test_memory(uint32_t base, uint32_t size)
{
    volatile uint32_t *ddr = (volatile uint32_t *)base;
    unsigned e;
    errors = 0;
    mem_base = base;
    printf("Memory tests at 0x%08x-0x%08x\n", base, base + size);

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
    printf("Memtest at 0x%08x (%u KiB)...\n", base, BLOCK >> 10);
    e = errors;
    printf("  Write: 0x%08x-0x%08x\n", base, base + BLOCK);
    clock_t t0 = cycle();
    for(uint32_t i = 0; i < n; i++)
        ddr[i] = ~(base + 4 * i);
    clock_t t_wr = cycle() - t0;
    printf("   Read: 0x%08x-0x%08x\n", base, base + BLOCK);
    t0 = cycle();
    for(uint32_t i = 0; i < n; i++)
        check(4 * i, ddr[i], ~(base + 4 * i));
    clock_t t_rd = cycle() - t0;
    puts(errors == e ? "Memtest OK\n" : "Memtest FAIL\n");

    printf("Memspeed at 0x%08x (sequential, %u KiB, %u MHz)...\n", base, BLOCK >> 10,
           platform_clk() / 1000000);
    print_speed("Write", BLOCK, t_wr);
    print_speed(" Read", BLOCK, t_rd);       // includes the compare
    return errors;
}

// exit code: 0 passed (or no DDR to test), 1 failed
int main()
{
    serial_init(UART_BAUD_115200);

    uint32_t size = platform_ddr_size();
    if(!size) {
        puts("ddr: this board has no DDR\n");
        return 0;
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
        return 1;
    }
    printf("done (waited %u us)\n", (unsigned)(waited / (platform_clk() / 1000000)));
    printf("  read delays: lane 0: %u, lane 1: %u (half-cycle steps, chosen by hardware)\n",
           calib & 7, (calib >> 3) & 7);

    unsigned failed = test_memory(SOC_DDR_ADDR + IMAGE_AREA, size - IMAGE_AREA);
    failed += test_image();

    if(failed)
        printf("FAIL: %u errors\n", failed);
    else
        puts("PASS\n");
    return failed != 0;
}
