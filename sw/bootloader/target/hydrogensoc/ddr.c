#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <mmio.h>

#include "common.h"
#include "ddr.h"

#ifdef SOC_EN_DDR

#define DDR_STATUS      REG32(SOC_DDR_CTRL_ADDR, 0x0)
#define DDR_CALIB       REG32(SOC_DDR_CTRL_ADDR, 0x4)
#define STATUS_READY    (1 << 0)
#define STATUS_FAIL     (1 << 1)
#define CALIB_TIMEOUT_MS 10             // calibration takes about 0.7 ms after reset

// Quick test, about 60 accesses: walking ones on the data bus, then one word per address
// bit so that aliasing shows. Returns the first failing offset + 1, or 0.
static uint32_t ddr_quicktest(uint32_t size) {
    volatile uint32_t *ddr = (volatile uint32_t *)SOC_DDR_ADDR;
    for(int b = 0; b < 32; b++) {
        ddr[0] = 1u << b;
        if(ddr[0] != 1u << b)
            return 1;
    }
    uint32_t nbits = 0;
    while((4u << nbits) < size)
        nbits++;
    for(uint32_t k = 0; k < nbits; k++)
        ddr[1u << k] = 0xA5000000 | k;
    ddr[0] = 0x5A5A5A5A;
    if(ddr[0] != 0x5A5A5A5A)
        return 1;
    for(uint32_t k = 0; k < nbits; k++)
        if(ddr[1u << k] != (0xA5000000 | k))
            return (4u << k) + 1;
    return 0;
}

// "calibrated (delays a b)" or "calibration FAILED", from a STATUS value
static void print_state(uint32_t st) {
    if(st & STATUS_FAIL) {
        puts("calibration FAILED");
        return;
    }
    uint32_t calib = DDR_CALIB;
    puts("calibrated (delays ");
    putchar('0' + (calib & 7)); putchar(' '); putchar('0' + ((calib >> 3) & 7));
    putchar(')');
}

void ddr_info() {
    uint32_t st = DDR_STATUS;
    puts("ddr: "); putnum(platform_ddr_size() >> 20, 10, 1); puts(" MiB, ");
    if(st & (STATUS_READY | STATUS_FAIL)) print_state(st);
    else puts("initializing");
    putchar('\n');
}

void ddr_check() {
    uint32_t size = platform_ddr_size();
    P(puts("ddr: ");)
    P(putnum(size >> 20, 10, 1);)
    P(puts(" MiB, ");)

    clock_t t_end = cycle() + (clock_t)(platform_clk() / 1000) * CALIB_TIMEOUT_MS;
    uint32_t st;
    while(!((st = DDR_STATUS) & (STATUS_READY | STATUS_FAIL)))
        if(cycle() > t_end) {
            P(puts("no response (calibration timeout)\n");)
            return;
        }
    P(print_state(st);)
    if(st & STATUS_FAIL) {
        P(putchar('\n');)
        return;
    }
    P(puts("\nddr: quick test - ");)

    uint32_t bad = ddr_quicktest(size);
    if(bad) {
        P(puts("FAILED at 0x");)
        P(putnum(SOC_DDR_ADDR + bad - 1, 16, 1);)
        P(putchar('\n');)
    } else {
        P(puts("OK\n");)
    }
}

#endif // SOC_EN_DDR
