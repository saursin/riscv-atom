#ifndef __PLATFORM_H__
#define __PLATFORM_H__

// ============== Target Independent Definitions ==============

// Bitmanip Macros
#define bitset(data, nbit)      ((data) |  (1<<(nbit)))
#define bitclear(data, nbit)    ((data) & ~(1<<(nbit)))
#define bitflip(data, nbit)     ((data) ^  (1<<(nbit)))
#define bitcheck(data, nbit)    ((data) &  (1<<(nbit)))
// #define bitsetv(data, nbit, val)(((data) &= ~(1<<(nbit))) | (val << nbit))


// ============== Target Specific Definitions ==============
#ifdef TARGET_HYDROGENSOC
    // #define EXCEPTION

    extern int __coderam_start;
    extern int __coderam_size;
    extern int __dataram_start;
    extern int __dataram_size;

    #define CODERAM_BASE    &__coderam_start
    #define CODERAM_SIZE    &__coderam_size

    #define DATARAM_BASE    &__dataram_start
    #define DATARAM_SIZE    &__dataram_size

    // ---------- UART ------------
    #define UART_ADDR 0x40000000
    
    // ---------- GPIO ------------
    #define GPIO
    #define GPIO_PINCOUNT 32
    #define GPIO_SETWIDTH 16
    #define GPIO_ADDR 0x40002000

    // ---------- SPI ------------
    #define SPI
    #define SPI_ADDR 0x40003000

    // ------ CLOCK FREQUENCY --------
    #define CLK_FREQ 12000000

#else
#ifdef TARGET_ATOMBONES
    extern int __coderam_start;
    extern int __coderam_size;
    extern int __dataram_start;
    extern int __dataram_size;

    #define CODERAM_BASE    &__coderam_start
    #define CODERAM_SIZE    &__coderam_size

    #define DATARAM_BASE    &__dataram_start
    #define DATARAM_SIZE    &__dataram_size


    // ---------- UART ------------
    #define UART_ADDR 0x40000000

    // ------ CLOCK FREQUENCY --------
    #define CLK_FREQ 12000000
#else
    #warning Must define a target for platform.h
#endif
#endif

#endif //__PLATFORM_H__