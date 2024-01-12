#pragma once

// ============== Target Independent Definitions ==============
#define N_STDDEV 4
#define DEV_STDIN  0
#define DEV_STDOUT 1
#define DEV_STDERR 2

// ============== Target Specific Definitions ==============
#ifdef TARGET_HYDROGENSOC
    #define EXCEPTION

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

    // --------- TIMER -----------
    #define TIMER_ADDR      0x40004000

    // ------ CLOCK FREQUENCY --------
    // Spartan6-mini
    // #define CLK_FREQ 12000000
    
    // DE0-CV
    #define CLK_FREQ 50000000

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
