#pragma once
#include <stdint.h>

#ifdef SOC_EN_UART
#ifdef EN_PRINTS
#define __EN_PRINTS
#endif
#endif

#ifdef __EN_PRINTS
    #define P(x) x
#else
    #define P(x)
#endif

#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

///////////////////////////////////////////////////////////////////////////////
// Return codes

// General
#define RCODE_OK                        0
#define RCODE_EXCEPTION                 1
#define RCODE_UNREACHABLE               2
#define RCODE_PLATFORM_INIT_FAIL        3

// Flashboot
#define RCODE_FLASHBOOT_NOSPI           10
#define RCODE_FLASHBOOT_INVALID_DEVICE  11

// UARTBOOT
#define RCODE_UARTBOOT_NOUART           20
#define RCODE_UARTBOOT_FAILED           21

// Generated from the soctarget/board config (cfgparse.py -t platform)
extern const uint32_t atom_platform_table[];

// val in base 10 or 16, at least `digits` digits (zero-padded)
void putnum(unsigned val, unsigned base, int digits);
void __attribute__((noreturn)) boot_panic(int code, char *msg);
void * platform_init();
