#ifndef __PLATFORM_H__
#define __PLATFORM_H__
#include "stddef.h"

// ============== Target Independent Definitions ==============

// Bitmanip Macros
#define bitset(data, nbit)      ((data) |  (1<<(nbit)))
#define bitclear(data, nbit)    ((data) & ~(1<<(nbit)))
#define bitflip(data, nbit)     ((data) ^  (1<<(nbit)))
#define bitcheck(data, nbit)    ((data) &  (1<<(nbit)))
// #define bitsetv(data, nbit, val)(((data) &= ~(1<<(nbit))) | (val << nbit))


// ============== Target Specific Definitions ==============
#ifdef TARGET_HYDROGENSOC
    #define MEM_ROM_BEGIN   0x00010000
    #define MEM_ROM_SIZE    32*1024        // 32 KB

    #define MEM_RAM_BEGIN   0x20000000
    #define MEM_RAM_SIZE    16*1024        // 16 KB

    // ---------- UART ------------
    #define UART_ADDR 0x40000000
    
    // ---------- GPIO ------------
    #define GPIO
    #define GPIO_PINCOUNT 32
    #define GPIO_SETWIDTH 16
    #define GPIO_ADDR 0x40002000

    // ------ CLOCK FREQUENCY --------
    #define CLK_FREQ 12000000

#else
#ifdef TARGET_ATOMBONES
    #define MEM_ROM_BEGIN   0x00000000
    #define MEM_ROM_SIZE    64*1024*1024    // 64 MB

    #define MEM_RAM_BEGIN   0x04000000
    #define MEM_RAM_SIZE    64*1024*1024    // 64 MB

    // ---------- UART ------------
    #define UART_D_REG_ADDR 0x08000000
    #define UART_S_REG_ADDR 0x08000001
    #define UART_CD_REG_ADDR 0x08000004

    // ------ CLOCK FREQUENCY --------
    #define CLK_FREQ 12000000
#else
    #warning Must define a target for platform.h
#endif
#endif

#endif //__PLATFORM_H__