#include <stdint.h>

#include "common.h"

#include <platform.h>
#include <stdio.h>
#include <time.h>
#include <gpio.h>
#include <spi.h>
#include "flashboot.h"

#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

extern uint32_t __approm_start;
extern uint32_t __approm_size;

//-------------------- Configuration --------------------
#define BOOTLED_PIN 0
#define BOOTMODE_PIN0 8
#define BOOTMODE_PIN1 9

// Address offset at which image resides
#define FLASH_IMG_OFFSET    0x000c0000

// Size of Image
#define FLASH_IMG_SIZE      32 * 1024       // 32 KB

// number of flashes to indicate entering of bootloader
#define START_LED_FLASHES   3

// number of flashes to indicate entering of user application
#define END_LED_FLASHES   1

// Bootmodes
#define BOOTMODE_FLASHBOOT      0
#define BOOTMODE_JUMP_TO_RAM    1
#define BOOTMODE_INF_LOOP       2
#define BOOTMODE_DEFAULT        BOOTMODE_JUMP_TO_RAM

//-------------------------------------------------------

/**
 * @brief Blink BOOTLED
 * 
 * @param pin led pin
 * @param count number of times to blink
 * @param time_period time_period in of one ON & OFF
 */
void led_blink(int pin, int count, int time_period)
{
    int halfperiod = time_period >> 1;
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
    led_blink(BOOTLED_PIN, START_LED_FLASHES, 50);

    // get bootmode
    uint8_t pin0_val = (uint8_t)gpio_read(BOOTMODE_PIN0);
    uint8_t pin1_val = (uint8_t)gpio_read(BOOTMODE_PIN1);
    bootmode = pin0_val | pin1_val << 1;
#else
    P(puts("nogpio: using default bootmode\n");)
#endif
    // Print Bootmode
    P(puts("bootmode: ");)
    P(putchar('0'+bootmode);)
    P(puts(": ");)
    
    // Perform action according to bootmode
    switch (bootmode)
    {
        case BOOTMODE_FLASHBOOT:
            P(puts("flashboot: copying from " EXPAND_AND_STRINGIFY(FLASH_IMG_OFFSET) "\n");)
        #ifdef SOC_EN_SPI
            flashcpy((uint8_t *)&__approm_start, FLASH_IMG_OFFSET, FLASH_IMG_SIZE);
        #else
            boot_panic(RCODE_FLASHBOOT_FAIL);
        #endif
            break;

        case BOOTMODE_JUMP_TO_RAM: 
            P(puts("jmptoram\n");)
            break;
    
        case BOOTMODE_INF_LOOP:
        default:
            P(puts("infloop\n");)
            while(1){
                asm volatile("");
            }
    }

    #ifdef SOC_EN_GPIO
    // Blink single (signal jump to user code)
    led_blink(BOOTLED_PIN, END_LED_FLASHES, 50);
    #endif
    
    // return start address of approm
    return (void *) &__approm_start;
}