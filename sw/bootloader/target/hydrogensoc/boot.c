#include <platform.h>
#include <stdint.h>
#include <stdio.h>
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

// Bootmodes
#define BOOTMODE_FLASHBOOT      0
#define BOOTMODE_JUMP_TO_RAM    1
#define BOOTMODE_UARTBOOT       2
#define BOOTMODE_INF_LOOP       3
#define BOOTMODE_DEFAULT        BOOTMODE_JUMP_TO_RAM

//-------------------------------------------------------

/**
 * @brief Toggle BOOTLED (progress indicator)
 */
void bootled_toggle()
{
#ifdef SOC_EN_GPIO
    static gpio_state_t state = HIGH;
    state = (state == HIGH) ? LOW : HIGH;
    gpio_write(BOOTLED_PIN, state);
#endif
}

void * platform_init(){
    uint8_t bootmode = BOOTMODE_DEFAULT;

#ifdef SOC_EN_GPIO
    // Initialize GPIO
    gpio_setmode(BOOTLED_PIN, OUTPUT);     
    gpio_setmode(BOOTMODE_PIN0, INPUT);
    gpio_setmode(BOOTMODE_PIN1, INPUT);

    // BOOTLED stays on while in the bootloader
    gpio_write(BOOTLED_PIN, HIGH);

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

    if(wait_for_keypress){
        puts("\nPress ENTER to continue...\n");
        while(serial_read()!='\r')
            asm volatile ("");
    }
    
    #ifdef SOC_EN_GPIO
    // BOOTLED off: jumping to user code
    gpio_write(BOOTLED_PIN, LOW);
    #endif
    
    // return start address of approm
    return (void *) &__approm_start;
}