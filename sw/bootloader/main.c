#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <serial.h>
#include <csr.h>

typedef void (*fnc_ptr)(void);

void puthex(unsigned val) {
    if(val == 0)
        return;
    puthex(val / 16);
    putchar("0123456789abcdef"[val % 16]);
}

void boot_panic_handler(){
    #ifdef __EN_PRINTS
    puts("Boot Exception: cause=0x"); puthex(CSR_read(CSR_MCAUSE));
    puts(", addr=0x"); puthex(CSR_read(CSR_MEPC));
    putchar('\n');
    #endif
    exit(RCODE_EXCEPTION);
}

void boot_panic(int code, char * msg){
#ifdef __EN_PRINTS
    puts("boot-panic: ");
    puts(msg ? msg : "?");
    putchar('\n');
#endif
    exit(code);
}

int main(){
    // ********* Platform agnostic initialization **********
    // Uart initialization
    P(serial_init(UART_BAUD_115200);)

    // Clear screen at start
    #ifdef CLS_AT_START
        P(putchar(0x1b); putchar('c');)  // clear screen
    #else
        P(putchar('\n'));
    #endif

    // Print header
    P(puts("**** RISC-V Atom Bootloader ****\n");)

    // ********** Platform specific initialization **********
    void * jump_addr = platform_init();
    if(jump_addr == NULL) {
        boot_panic(RCODE_PLATFORM_INIT_FAIL, "Platform init failure");
    }

    // Jump to target
    P(puts("--------------------------------\n");)
    fnc_ptr app_main = (fnc_ptr) jump_addr;
    app_main();

    // Unreachable
    boot_panic(RCODE_UNREACHABLE, "Unreachable");
}