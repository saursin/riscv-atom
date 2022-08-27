#include "defs.h"
#include "gpio.h"

// compile only for Hydrogensoc
#ifdef TARGET_HYDROGENSOC

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
    // Reset all GPIO sets
    for (unsigned int set=0; set<(GPIO_PINCOUNT/GPIO_SETWIDTH); set++)
    {
        *((volatile short*) (GPIO_ADDR+(4*set+2))) = 0x0000;  // Set as output
        *((volatile short*) (GPIO_ADDR+(4*set))) = 0x0000;  // Pull Low
    }
}


/**
 * @brief Read the current GPIO pin values
 * 
 * @param pin pin number
 * @return gpio_state pin value
 */
gpio_state gpio_read(int pin)
{
    int gpio_set = pin / GPIO_SETWIDTH;
    int pin_index = pin % GPIO_SETWIDTH;

    int addr_offset = 4*gpio_set;

    unsigned short gpio_set_val = *((volatile short*) (GPIO_ADDR+addr_offset));
    return (gpio_set_val & (1 << pin_index)) ? HIGH : LOW;
}


/**
 * @brief Write to a GPIO pin
 * 
 * @param pin pin number
 * @param state state
 */
void gpio_write(int pin, gpio_state state)
{
    int gpio_set = pin / GPIO_SETWIDTH;
    int pin_index = pin % GPIO_SETWIDTH;

    int addr_offset = 4*gpio_set;

    unsigned short gpio_set_val = *((volatile short*) (GPIO_ADDR+addr_offset));
    gpio_set_val = gpio_set_val & ~(1 << pin_index);
    *((volatile short*) (GPIO_ADDR+addr_offset)) = gpio_set_val | ((state==HIGH) ? (0x1 << pin_index) : 0x0);
}


/**
 * @brief Set mode of a GPIO pin
 * 
 * @param pin pin number
 * @param mode mode (INPUT/OUTPUT)
 */
void gpio_setmode(int pin, gpio_direction mode)
{
    int gpio_set = pin / GPIO_SETWIDTH;
    int pin_index = pin % GPIO_SETWIDTH;

    int addr_offset = (4*gpio_set)+2;   // additionl offset of 2 bytes to access pinmode register

    unsigned short gpio_set_val = *((volatile short*) (GPIO_ADDR+addr_offset));
    gpio_set_val = gpio_set_val & ~(1 << pin_index);
    *((volatile short*) (GPIO_ADDR+addr_offset)) = gpio_set_val | ((mode==INPUT) ? (0x1 << pin_index) : 0x0);
}


/**
 * @brief Get mode of a GPIO pin
 * 
 * @param pin gpio pin
 * @return gpio_direction 
 */
gpio_direction gpio_getmode(int pin)
{
    int gpio_set = pin / GPIO_SETWIDTH;
    int pin_index = pin % GPIO_SETWIDTH;

    int addr_offset = (4*gpio_set)+2;   // additionl offset of 2 bytes to access pinmode register

    unsigned short gpio_set_val = *((volatile short*) (GPIO_ADDR+addr_offset));
    return (gpio_set_val & (1 << pin_index)) ? INPUT : OUTPUT;
}
#endif