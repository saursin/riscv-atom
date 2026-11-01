// Bootloader console (bootmode 3, or after a failed boot): a few commands to look around,
// poke memory and start a program.
#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <csr.h>
#include <xmodem.h>

#include "common.h"
#include "console.h"
#include "uartboot.h"
#include "flashboot.h"
#include "ddr.h"

extern uint32_t __approm_start;
extern uint32_t __approm_size;

// "NAME  <size> @ 0x<addr>"
static void region(const char *name, uint32_t addr, uint32_t size) {
    puts(name);
    if(size >= (1u << 20)) { putnum(size >> 20, 10, 1); puts(" MiB"); }
    else                   { putnum(size >> 10, 10, 1); puts(" KiB"); }
    puts(" @ 0x"); putnum(addr, 16, 8); putchar('\n');
}

static void info() {
    uint32_t misa = CSR_read(CSR_MISA);
    puts("CPU:     Atom rv32");                 // ISA from misa: base, then extensions in ISA order
    putchar(misa & (1u << ('E' - 'A')) ? 'e' : 'i');
    for(const char *x = "MAFDQCBV"; *x; x++)
        if(misa & (1u << (*x - 'A')))
            putchar(*x | 0x20);
    puts(" @ "); putnum(platform_clk() / 1000000, 10, 1); puts(" MHz\n");
    region("BOOTROM: ", SOC_BOOTROM_ADDR, SOC_BOOTROM_SIZE);
    region("RAM:     ", SOC_RAM_ADDR, SOC_RAM_SIZE);
#ifdef SOC_EN_DDR
    region("DDR:     ", SOC_DDR_ADDR, platform_ddr_size());
#endif
    puts("IO:     ");
#ifdef SOC_EN_UART
    puts(" UART");
#endif
#ifdef SOC_EN_GPIO
    puts(" GPIO");
#endif
#ifdef SOC_EN_SPI
    puts(" SPI");
#endif
#ifdef SOC_EN_TIMER
    puts(" TIMER");
#endif
    putchar('\n');
}

static int streq(const char *a, const char *b) {
    while(*a && *a == *b) { a++; b++; }
    return *a == *b;
}

// hex with 0x, else decimal; *ok cleared on a bad digit
static uint32_t parse_num(const char *s, int *ok) {
    uint32_t v = 0, base = 10;
    if(s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
    if(!*s) *ok = 0;
    for(; *s; s++) {
        uint32_t d = (*s >= '0' && *s <= '9') ? *s - '0'
                   : ((*s | 0x20) >= 'a' && (*s | 0x20) <= 'f') ? (*s | 0x20) - 'a' + 10 : 99;
        if(d >= base) { *ok = 0; return 0; }
        v = v * base + d;
    }
    return v;
}

// line input with echo and backspace
static void readline(char *buf, int n) {
    int len = 0;
    for(;;) {
        char c = getchar();
        if(c == '\r' || c == '\n') break;
        if((c == 8 || c == 127) && len) { len--; puts("\b \b"); }
        else if(c >= ' ' && len < n - 1) { buf[len++] = c; putchar(c); }
    }
    buf[len] = 0;
    putchar('\n');
}

static const char help[] =
    "help                  this list\n"
    "info                  SoC summary\n"
    "clear                 clear the screen\n"
    "md <addr> [words]     dump memory\n"
    "mw <addr> <value>     write a word\n"
    "fill <addr> <len> <v> fill memory with a word\n"
    "crc <addr> <len>      CRC-16/XMODEM of memory\n"
    "memtest <addr> <len>  address-in-address test\n"
#ifdef SOC_EN_DDR
    "ddr                   DDR size and calibration\n"
#endif
    "load [addr]           receive a file (XMODEM) at addr, default program RAM\n"
    "save <addr> <len> [name] send memory as a file (XMODEM)\n"
#ifdef SOC_EN_SPI
    "flashboot             load a program from flash and run it\n"
#endif
    "boot [addr]           run the program in RAM (or at addr)\n"
    "reboot\n";

void *console() {
    puts("=========== Console ============\n");
    info();
    puts("type 'help' for commands\n");
    char line[48];
    for(;;) {
        puts("\e[0;34matom-boot: \e[0m");
        readline(line, sizeof line);

        // split into at most 4 words
        char *argv[4] = {0, 0, 0, 0};
        int argc = 0;
        for(char *p = line; *p && argc < 4; ) {
            while(*p == ' ') *p++ = 0;
            if(!*p) break;
            argv[argc++] = p;
            while(*p && *p != ' ') p++;
        }
        if(!argc) continue;

        int ok = 1;
        uint32_t a1 = argc > 1 ? parse_num(argv[1], &ok) : 0;
        uint32_t a2 = argc > 2 ? parse_num(argv[2], &ok) : 0;
        uint32_t a3 = argc > 3 && !streq(argv[0], "save") ? parse_num(argv[3], &ok) : 0;
        char *cmd = argv[0];
        if(!ok) {
            puts("bad number\n");
        } else if(streq(cmd, "help")) {
            puts(help);
        } else if(streq(cmd, "info")) {
            info();
        } else if(streq(cmd, "clear")) {
            puts("\033[H\033[2J");             // cursor home, erase screen
        } else if(streq(cmd, "md") && argc > 1) {
            volatile uint32_t *p = (volatile uint32_t *)(a1 & ~3u);
            uint32_t n = argc > 2 ? a2 : 16;
            for(uint32_t i = 0; i < n; i++) {
                if(i % 4 == 0) { putnum((uint32_t)(p + i), 16, 8); putchar(':'); }
                putchar(' '); putnum(p[i], 16, 8);
                if(i % 4 == 3 || i == n - 1) putchar('\n');
            }
        } else if(streq(cmd, "mw") && argc > 2) {
            *(volatile uint32_t *)(a1 & ~3u) = a2;
        } else if(streq(cmd, "fill") && argc > 3) {
            volatile uint32_t *p = (volatile uint32_t *)(a1 & ~3u);
            for(uint32_t i = 0; i < a2 / 4; i++) p[i] = a3;
        } else if(streq(cmd, "crc") && argc > 2) {
            putnum(xmodem_crc16((const uint8_t *)a1, a2), 16, 4); putchar('\n');
        } else if(streq(cmd, "memtest") && argc > 2) {
            volatile uint32_t *p = (volatile uint32_t *)(a1 & ~3u);
            uint32_t n = a2 / 4, errors = 0;
            for(uint32_t i = 0; i < n; i++) p[i] = ~(uint32_t)(p + i);
            for(uint32_t i = 0; i < n; i++)
                if(p[i] != ~(uint32_t)(p + i) && !errors++) { puts("first error @0x"); putnum((uint32_t)(p + i), 16, 8); putchar('\n'); }
            if(errors) { putnum(errors, 10, 1); puts(" errors\n"); }
            else puts("OK\n");
#ifdef SOC_EN_DDR
        } else if(streq(cmd, "ddr")) {
            ddr_info();
#endif
        } else if(streq(cmd, "load")) {
            uint32_t at = argc > 1 ? a1 : (uint32_t)&__approm_start, max = 0;
            uint32_t ram = (uint32_t)&__approm_start, ram_end = ram + (uint32_t)&__approm_size;
            uint32_t ddr_end = SOC_DDR_ADDR + platform_ddr_size();
            if(at >= ram && at < ram_end) max = ram_end - at;
            else if(at >= SOC_DDR_ADDR && at < ddr_end) max = ddr_end - at;
            if(!max) {
                puts("not in RAM or DDR\n");
            } else {
                int n = xmodem_recv((uint8_t *)at, max);
                putchar('\n');
                if(n < 0) { puts("load failed ("); putnum(-n, 10, 1); puts(")\n"); }
                else { putnum(n, 10, 1); puts(" bytes at 0x"); putnum(at, 16, 8); putchar('\n'); }
            }
        } else if(streq(cmd, "save") && argc > 2) {
            int r = xmodem_send(argc > 3 ? argv[3] : "blob.bin", (const void *)a1, a2);
            putchar('\n');
            puts(r ? (r == XMODEM_ERR_TIMEOUT ? "no receiver\n" : "save failed\n") : "saved\n");
#ifdef SOC_EN_SPI
        } else if(streq(cmd, "flashboot")) {
            if(flashboot((uint8_t *)&__approm_start, FLASH_IMG_OFFSET, (unsigned)&__approm_size) == 0)
                return &__approm_start;
#endif
        } else if(streq(cmd, "boot")) {
            return argc > 1 ? (void *)a1 : (void *)&__approm_start;
        } else if(streq(cmd, "reboot")) {
            ((void (*)(void))SOC_BOOTROM_ADDR)();      // the reset vector
        } else {
            puts("? (help)\n");
        }
    }
}
