#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_MAX_PINS 4
#define GPIO_ADDR 0x08000100
#define GPIO_HIGH 1
#define GPIO_LOW 0

/**
 * @brief Resets all gpio pins to LOW state
 */
void gpio_reset()
{
    // set all gpio pins to zero
    int addr;
    for(addr = GPIO_ADDR; addr<GPIO_ADDR+GPIO_MAX_PINS; addr++)
    {
        *((volatile char*) addr) = GPIO_LOW;
    }
    return;
}

/**
 * @brief Initialize GPIO
 * 
 */
void gpio_init()
{
    return gpio_reset();
}

/**
 * @brief read current gpio value
 * 
 * @param pin pin number
 * @return int pin value
 */
int gpio_get(int pin)
{
    int pin_address = GPIO_ADDR + pin;
    return *((volatile char*) pin_address);
}

/**
 * @brief Write to a GPIO pin
 * 
 * @param pin pin number
 * @param state value
 */
void gpio_set(int pin, char state)
{
    int pin_address = GPIO_ADDR + pin;
    *((volatile char*) pin_address) = state;
    return;
}

#endif // __GPIO_H__