#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <serial.h>
#include <csr.h>

extern void _reenter(void);

#define SEPARATOR "--------------------------------\n"

// Jumps to a program: a0 = hart id, a1 = platform table, ra = _reenter (where its _exit returns)
static void __attribute__((noreturn)) start_program(void *entry) {
    register long a0 asm("a0") = 0;
    register const uint32_t *a1 asm("a1") = atom_platform_table;
    asm volatile("la ra, _reenter\n"
                 "jr %0" :: "r"(entry), "r"(a0), "r"(a1) : "ra", "memory");
    __builtin_unreachable();
}

void putnum(unsigned val, unsigned base, int digits) {
    if(val >= base || digits > 1)
        putnum(val / base, base, digits - 1);
    putchar("0123456789abcdef"[val % base]);
}

void boot_panic_handler(){
    #ifdef __EN_PRINTS
    puts("Boot Exception: cause=0x"); putnum(CSR_read(CSR_MCAUSE), 16, 1);
    puts(", addr=0x"); putnum(CSR_read(CSR_MEPC), 16, 1);
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

// kind: BOOT_* (common.h), the kind of boot; arg: a parameter for that kind
int main(int kind, int arg){
    // ********* Platform agnostic initialization **********
    // Platform table (clock, ...) for libcatom, here and in the program
    __platform = atom_platform_table;

    // Uart initialization
    P(serial_init(UART_BAUD_115200);)

    if(kind == BOOT_REENTRY) {
        // arg: the program's exit code, in decimal
        P(putchar('\n'); puts(SEPARATOR); puts("program exited (");)
        P(if(arg < 0) putchar('-');)
        P(putnum(arg < 0 ? -(unsigned)arg : (unsigned)arg, 10, 1);)
        P(puts(")\n");)
    } else {
        // Clear screen at start
        #ifdef CLS_AT_START
            P(putchar(0x1b); putchar('c');)  // clear screen
        #else
            P(putchar('\n'));
        #endif

        // Print header
        P(puts("**** RISC-V Atom Bootloader ****\n");)
    }

    // ********** Platform specific part **********
    void * entry = platform_boot(kind, arg);
    if(entry == NULL) {
        boot_panic(RCODE_PLATFORM_INIT_FAIL, "Nothing to run");
    }

    // Jump to target
    P(puts(SEPARATOR);)
    start_program(entry);
}
