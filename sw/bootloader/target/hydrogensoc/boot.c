#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <gpio.h>

#include "common.h"
#include "flashboot.h"
#include "uartboot.h"

extern uint32_t __approm_start;
extern uint32_t __approm_size;

//-------------------- Configuration --------------------
#define BOOTLED_PIN 0
#define BOOTMODE_PIN0 8
#define BOOTMODE_PIN1 9

// Address offset at which image resides
#define FLASH_IMG_OFFSET    0x000c0000

// number of flashes to indicate entering of bootloader
#define START_LED_FLASHES   3

// number of flashes to indicate entering of user application
#define END_LED_FLASHES   1

// Time period of LED flashes
#define LED_FLASH_TIMEPERIOD 100

// Bootmodes
#define BOOTMODE_FLASHBOOT      0
#define BOOTMODE_JUMP_TO_RAM    1
#define BOOTMODE_UARTBOOT       2
#define BOOTMODE_INF_LOOP       3
#define BOOTMODE_DEFAULT        BOOTMODE_JUMP_TO_RAM

//-------------------------------------------------------

/**
 * @brief Blink BOOTLED
 * 
 * @param pin led pin
 * @param count number of times to blink
 * @param period_ms time_period in of one ON & OFF
 */
void led_blink(int pin, int count, int period_ms)
{
    int halfperiod = period_ms >> 1;
    #ifdef SIM
    halfperiod = 0;
    #endif

    for(int i=0; i<count; i++) 
    {
        gpio_write(pin, HIGH);
        sleep_ms(halfperiod);
        gpio_write(pin, LOW);
        sleep_ms(halfperiod);
    }
}

void * platform_init(){
    uint8_t bootmode = BOOTMODE_DEFAULT;

#ifdef SOC_EN_GPIO
    // Initialize GPIO
    gpio_setmode(BOOTLED_PIN, OUTPUT);     
    gpio_setmode(BOOTMODE_PIN0, INPUT);
    gpio_setmode(BOOTMODE_PIN1, INPUT);

    // Blink LED START_LED_FLASHES times (signal entering of bootloader)
    led_blink(BOOTLED_PIN, START_LED_FLASHES, LED_FLASH_TIMEPERIOD);

    // get bootmode
    uint8_t pin0_val = (uint8_t)gpio_read(BOOTMODE_PIN0);
    uint8_t pin1_val = (uint8_t)gpio_read(BOOTMODE_PIN1);
    bootmode = pin0_val | pin1_val << 1;
#else
    P(puts("no gpio: using default bootmode\n");)
#endif

    // Print Bootmode
    P(puts("bootmode: 0x");)
    P(putchar('0'+bootmode);)
    P(putchar('\n');)
    
    // Perform action according to bootmode
    bool wait_for_keypress = false;
    switch (bootmode)
    {
        case BOOTMODE_FLASHBOOT:
        #ifdef SOC_EN_SPI
            flashboot((uint8_t *)&__approm_start, FLASH_IMG_OFFSET, (unsigned)&__approm_size);
        #else
            boot_panic(RCODE_FLASHBOOT_NOSPI, "No SPI IP");
        #endif
            break;

        case BOOTMODE_JUMP_TO_RAM: 
            P(puts("Jumping to RAM\n");)
            break;
        
        case BOOTMODE_UARTBOOT:
            wait_for_keypress = true;
        #ifdef SOC_EN_UART
            uartboot((uint8_t *)&__approm_start, (unsigned)&__approm_size);
        #else
            boot_panic(RCODE_UARTBOOT_NOUART, "No UART IP");  // TODO: fixme
        #endif
            break;
    
        case BOOTMODE_INF_LOOP:
        default:
            P(puts("Infinite loop\n");)
            while(1){
                asm volatile("");
            }
    }

    #ifdef SIM
    wait_for_keypress = false;
    #endif

    if(wait_for_keypress){
        puts("\nPress ENTER to continue...\n");
        while(serial_read()!='\r')
            asm volatile ("");
    }
    
    #ifdef SOC_EN_GPIO
    // Blink single (signal jump to user code)
    led_blink(BOOTLED_PIN, END_LED_FLASHES, LED_FLASH_TIMEPERIOD);
    #endif
    
    // return start address of approm
    return (void *) &__approm_start;
}