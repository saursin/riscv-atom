#include "platform.h"
#include <stdio.h>
#include <csr.h>
#include  <gpio.h>
#include <time.h>

char * banner = 
"\n"
"                  .';,.           ....;;;.  \n"
"                 .ll,:o,                ':c,. \n"
"                 .dd;co'                  .cl,  \n"
"                .:o:;,.                     'o:  \n"
"                co.                          .oc  \n"
"               ,o'          .coddoc.          'd,  \n"
"               lc         .lXMMMMMMXl.         ll  \n"
"              .o:         ;KMMMMMMMMK,         :o. \n"
"              .o:         'OMMMMMMMMO.         :o. \n"
"               co.         .o0XNNX0o.         .oc  \n"
"               .o:           ..''..           :o.  \n"
"                'o:                          :o'  \n"
"                 .lc.                      .ll.  \n"
"                   ,lc'                  'cl,   \n"
"                     'cc:,..        ..,:c:'   \n"
"                        .;::::;;;;::::;.    \n"
"                              ....        \n"
"     ____  _________ _______    __         __                 \n"
"    / __ \\/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___  \n"
"   / /_/ // / \\__ \\/ /    | | / /  / __ `/ __/ __ \\/ __ `__ \\ \n"
"  / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /      \n"
" /_/ |_/___//____/\\____/  |___/   \\__,_/\\__/\\____/_/ /_/ /_/  \n"
"/=========By: Saurabh Singh (saurabh.s99100@gmail.com)====/\n\n";

extern int _etext, _sdata, _ebss;     // linker script: end of code, start of data, end of bss

// prints a size as KiB or MiB
static void print_size(uint32_t bytes) {
    if(bytes >= (1 << 20))
        printf("%d MiB", (int)(bytes >> 20));
    else
        printf("%d KiB", (int)(bytes >> 10));
}

static void print_region(const char *name, uint32_t addr, uint32_t size) {
    printf("%s: 0x%08x  ", name, (unsigned)addr);
    print_size(size);
    putchar('\n');
}

// ISA string from misa: base, standard extensions in canonical order, then Zicsr (misa is a CSR)
static void print_isa() {
    uint32_t misa = CSR_read(CSR_MISA);
    puts("rv32");
    putchar(bitcheck(misa, 'E' - 'A') ? 'e' : 'i');
    const char *ext = "MAFDC";
    for(int i = 0; ext[i]; i++)
        if(bitcheck(misa, ext[i] - 'A'))
            putchar(ext[i] - 'A' + 'a');
    puts("_zicsr");
}

int main() {
    puts(banner);

    uint32_t clk = platform_clk();
    puts("CPU      : RISC-V Atom");
    if(clk)
        printf(" @ %d MHz", (int)(clk / 1000000));
    printf(" (hart %d)\n", (int)CSR_read(CSR_MHARTID));
    puts("ISA      : "); print_isa(); putchar('\n');
    printf("IDs      : vendor 0x%x, arch 0x%x, impl 0x%x\n", (unsigned)CSR_read(CSR_MVENDORID),
           (unsigned)CSR_read(CSR_MARCHID), (unsigned)CSR_read(CSR_MIMPID));
    printf("Config   : %s\n", SOC_CONFIG);

    print_region("BOOTROM  ", SOC_BOOTROM_ADDR, SOC_BOOTROM_SIZE);
    print_region("RAM      ", SOC_RAM_ADDR, SOC_RAM_SIZE);
    print_region("  code   ", (uint32_t)CODERAM_BASE, (uint32_t)CODERAM_SIZE);
    print_region("  data   ", (uint32_t)DATARAM_BASE, (uint32_t)DATARAM_SIZE);
#ifdef SOC_DDR_ADDR
    if(platform_ddr_size())
        print_region("DDR      ", SOC_DDR_ADDR, platform_ddr_size());
#endif

#ifdef SOC_EN_UART
    printf("UART     : 0x%08x  FIFO depth %d\n", (unsigned)SOC_UART_ADDR, SOC_UART_FIFO_DEPTH);
#else
    printf("UART     : 0x%08x\n", (unsigned)SOC_UART_ADDR);
#endif
#ifdef SOC_EN_GPIO
    printf("GPIO     : 0x%08x  %d pins\n", (unsigned)SOC_GPIO_ADDR, SOC_GPIO_NUM_PINS);
#endif
#ifdef SOC_EN_SPI
    printf("SPI      : 0x%08x  %d chip select(s)\n", (unsigned)SOC_SPI_ADDR, SOC_SPI_NUM_CS);
#endif
#ifdef SOC_EN_TIMER
    printf("TIMER    : 0x%08x\n", (unsigned)SOC_TIMER_ADDR);
#endif

    printf("Program  : %d bytes code, %d bytes data\n",
           (int)((uint32_t)&_etext - (uint32_t)CODERAM_BASE), (int)((uint32_t)&_ebss - (uint32_t)&_sdata));
    printf("Cycles   : %d since reset\n", (int)cycle());
    if(!clk)
        puts("(no platform table: not started by the bootloader, so the clock is unknown)\n");
    return 0;
}
