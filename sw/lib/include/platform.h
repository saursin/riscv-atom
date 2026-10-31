#pragma once
#include <stdint.h>

// ============== Target Independent Definitions ==============
#define N_STDDEV 4
#define DEV_STDIN  0
#define DEV_STDOUT 1
#define DEV_STDERR 2

// ============== Memory Map ==============
// SOC_<X>_ADDR / SOC_<X>_SIZE come from the soctarget config (rtl/config/<soctarget>.yml)
// as -D defines; RAM layout symbols come from the linker script.
extern int __coderam_start;
extern int __coderam_size;
extern int __dataram_start;
extern int __dataram_size;

#define CODERAM_BASE    &__coderam_start
#define CODERAM_SIZE    &__coderam_size

#define DATARAM_BASE    &__dataram_start
#define DATARAM_SIZE    &__dataram_size

// ============== Platform table ==============
// Board facts generated from the soctarget/board config (cfgparse.py -t platform) into
// the boot ROM; the bootloader passes its address to the program in a1.
// Layout (little-endian u32 words):
//   header: magic "ATOM", version, size (bytes, header and tags included)
//   tags:   fourcc, size (bytes, tag header included, multiple of 4), payload
// Readers skip unknown tags; tags only grow at the end, so check a tag's size
// before reading a field. The version changes only on an incompatible change.
#define PLATFORM_MAGIC      0x4d4f5441      // "ATOM"
#define PLATFORM_VERSION    1
#define PLATFORM_TAG(a, b, c, d) \
    ((uint32_t)(a) | (uint32_t)(b) << 8 | (uint32_t)(c) << 16 | (uint32_t)(d) << 24)
#define PLATFORM_TAG_CLK    PLATFORM_TAG('C', 'L', 'K', ' ')   // clock (Hz)
#define PLATFORM_TAG_DDR    PLATFORM_TAG('D', 'D', 'R', ' ')   // DDR memory size (bytes), at SOC_DDR_ADDR

extern const uint32_t *__platform;          // the table, or NULL (set at startup)

// n-th tag (0 = first) with the given fourcc, or NULL
const uint32_t *platform_find(uint32_t tag, unsigned n);

// SoC clock in Hz from the platform table, or 0 without one
uint32_t platform_clk(void);

// DDR memory size in bytes (at SOC_DDR_ADDR), or 0 if the board has none
uint32_t platform_ddr_size(void);
