#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <gpio.h>

#include "common.h"
#include "flashboot.h"
#include "uartboot.h"
#include "ddr.h"
#include "console.h"

extern uint32_t __approm_start;
extern uint32_t __approm_size;

//-------------------- Configuration --------------------
#define BOOTLED_PIN 0
#define BOOTMODE_PIN0 8
#define BOOTMODE_PIN1 9

// Bootmodes
#define BOOTMODE_FLASHBOOT      0
#define BOOTMODE_JUMP_TO_RAM    1
#define BOOTMODE_UARTBOOT       2
#define BOOTMODE_CONSOLE        3
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
    void *jump_addr = &__approm_start;

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

#ifdef SOC_EN_DDR
    ddr_check();
#endif

    // Print Bootmode
    P(puts("bootmode: 0x");)
    P(putchar('0'+bootmode);)
    P(putchar('\n');)
    
    // Perform action according to bootmode
    switch (bootmode)
    {
        case BOOTMODE_FLASHBOOT:
        #ifdef SOC_EN_SPI
            if(flashboot((uint8_t *)&__approm_start, FLASH_IMG_OFFSET, (unsigned)&__approm_size))
                jump_addr = console();
        #else
            jump_addr = console();
        #endif
            break;

        case BOOTMODE_JUMP_TO_RAM: 
            P(puts("Jumping to RAM\n");)
            break;
        
        case BOOTMODE_UARTBOOT:
        #ifdef SOC_EN_UART
            if(uartboot((uint8_t *)&__approm_start, (unsigned)&__approm_size))
                jump_addr = console();      // failed: let the user retry or look around
        #else
            boot_panic(RCODE_UARTBOOT_NOUART, "No UART IP");
        #endif
            break;

        case BOOTMODE_CONSOLE:
        default:
            jump_addr = console();
    }

    #ifdef SOC_EN_GPIO
    // BOOTLED off: jumping to user code
    gpio_write(BOOTLED_PIN, LOW);
    #endif
    
    return jump_addr;
}