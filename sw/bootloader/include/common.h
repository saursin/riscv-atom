#pragma once

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

// Return codes
#define RCODE_OK                0
#define RCODE_EXCEPTION         1
#define RCODE_UNREACHABLE       2
#define RCODE_INIT_FAIL         3
#define RCODE_FLASHBOOT_FAIL    4

void puthex(unsigned val);

void __attribute__((noreturn)) boot_panic(int code);

void * platform_init();
