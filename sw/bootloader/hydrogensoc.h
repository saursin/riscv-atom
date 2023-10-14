#pragma once

#include "gpio.h"
#include "time.h"
#include "mmio.h"
#include "spi.h"

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
    for(int i=0; i<count; i++) 
    {
        gpio_write(pin, HIGH);
        sleep_ms(time_period >> 1);
        gpio_write(pin, LOW);
        sleep_ms(time_period >> 1);
    }
}


/**
 * @brief Platform specific initialization
 * 
 * @return void* address to jump to after boot
 */
void * platform_init() {
    // Initialize GPIO
    gpio_setmode(BOOTLED_PIN, OUTPUT);     
    gpio_setmode(BOOTMODE_PIN0, INPUT);
    gpio_setmode(BOOTMODE_PIN1, INPUT);

    // Blink LED START_LED_FLASHES times (signal entering of bootloader)
    led_blink(BOOTLED_PIN, START_LED_FLASHES, 50);

    // get bootmode
    uint8_t bootmode = (uint8_t)gpio_read(BOOTMODE_PIN0)
                        | (((uint8_t)gpio_read(BOOTMODE_PIN1)) << 1);
    /**
     * Bootmode: 
     *  0b00: flashboot
     *  0b01: jump to ram
     *  default: infinite loop
    */
    if (bootmode == 0b00) {
        // Bootmode: flash boot
        P(puts("flashboot: Copying from " EXPAND_AND_STRINGIFY(FLASH_IMG_OFFSET) "\n");)
        // copy from flash
        flashcpy((uint8_t *)&__approm_start, FLASH_IMG_OFFSET, FLASH_IMG_SIZE);
    } else if (bootmode == 0b01) {
        // Bootmode: jump to ram
    } else { 
        // Bootmode: Infinite loop
        while(1){
            asm volatile("");
        }
    }

    // Blink single (signal jump to user code)
    led_blink(BOOTLED_PIN, END_LED_FLASHES, 50);

    // Jump to start of approm
    return (void *) &__approm_start;
}
