#include "defs.h"
#include "gpio.h"

/**
 * @brief Initialize GPIO
 */
void gpio_init()
{
    return gpio_reset();
}


/**
 * @brief Resets all gpio pins to LOW state
 */
void gpio_reset()
{
    // set all gpio pins to zero
    int addr;
    for(addr = GPIO_ADDR; addr<GPIO_ADDR+GPIO_PINCOUNT; addr++)
    {
        *((volatile char*) addr) = GPIO_LOW;
    }
    return;
}


/**
 * @brief Read the current GPIO pin values
 * 
 * @param pin pin number
 * @return gpio_state pin value
 */
int gpio_read(int pin)
{
    int pin_address = GPIO_ADDR + pin;
    return *((volatile char*) pin_address);
}


/**
 * @brief Write to a GPIO pin
 * 
 * @param pin pin number
 * @param state state
 */
void gpio_write(int pin, int state)
{
    int pin_address = GPIO_ADDR + pin;
    *((volatile char*) pin_address) = state;
    return;
}


/**
 * @brief Set mode of a GPIO pin
 * 
 * @param pin pin number
 * @param mode mode (INPUT/OUTPUT)
 */
// void gpio_pinmode(int pin, int mode)
// {
//          UNIMPLEMENTED
// }
