#ifndef __DEFS_H__
#define __DEFS_H__

// ============== Target Independent Definitions ==============
// Useful Macros
#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

// ============== Target Specific Definitions ==============
#ifdef TARGET_HYDROGENSOC
    #define MEM_ROM_BEGIN   0x00000000
    #define MEM_ROM_SIZE    32*1024         // 64 KB

    #define MEM_RAM_BEGIN   0x04000000
    #define MEM_RAM_SIZE    8*1024         // 8 KB

    // ---------- UART ------------
    #define UART_D_REG_ADDR 0x08000000
    #define UART_S_REG_ADDR 0x08000001
    #define UART_CD_REG_ADDR 0x08000004

    // ---------- GPIO ------------
    #define GPIO
    #define GPIO_PINCOUNT 32
    #define GPIO_SETWIDTH 16
    #define GPIO_ADDR 0x08000100

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
    #warning Must define a target for defs.h
#endif
#endif

#endif //__DEFS_H__