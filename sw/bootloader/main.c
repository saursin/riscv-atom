#include <stdint.h>
#include "platform.h"
#include "stdio.h"

// Enable Prints through UART
#define ENABLE_UART

// Clear screen at start of bootloader
#define CLS_AT_START

#ifdef ENABLE_UART
    #define P(x) x
#else
    #define P(x)
#endif

// include target specific header
#include TARGET_HEADER

typedef void (*fnc_ptr)(void);

int main() {
    //********* Platform agnostic initialization **********
    serial_init(UART_BAUD_115200);


    // Print header
    #ifdef CLS_AT_START
    P(putchar(0x1b); putchar('c');)  // clear screen
    #else
    P(putchar('\n');)
    #endif
    P(puts("***** RISC-V Atom Bootloader *****\n");)


    // ********** Platform specific initialization **********
    void * jump_addr = platform_init();
    if(jump_addr == NULL) {
        P(puts("boot-panic: platform initialization failed\n");)
        goto panic;
    }

    // Jump
    fnc_ptr app_main = (fnc_ptr) jump_addr;
    app_main();


panic:
    P(puts("boot-panic: failed to boot\n");)
    while(1) {
        asm volatile("");
    }
}