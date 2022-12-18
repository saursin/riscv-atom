#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LOW=0, 
    HIGH=1
} gpio_state_t;

typedef enum {
    OUTPUT=0, 
    INPUT=1
} gpio_direction_t;

/**
 * @brief Initialize GPIO
 */
void gpio_init();


/**
 * @brief Resets all gpio pins to LOW state
 */
void gpio_reset();


/**
 * @brief Read the current GPIO pin value
 * 
 * @param pin pin number
 * @return gpio_state_t pin value
 */
gpio_state_t gpio_read(int pin);


/**
 * @brief Read the current GPIO pins
 * 
 * @return gpio_state_t pin values
 */
uint32_t gpio_readw();

/**
 * @brief Write to a GPIO pin
 * 
 * @param pin pin number
 * @param state state
 */
void gpio_write(int pin, gpio_state_t state);

/**
 * @brief Write to GPIO pins
 * 
 * @param state pin values
 */
void gpio_writew(uint32_t state);

/**
 * @brief Set mode of a GPIO pin
 * 
 * @param pin pin number
 * @param mode mode (INPUT/OUTPUT)
 */
void gpio_setmode(int pin, gpio_direction_t mode);


/**
 * @brief Get mode of a GPIO pin
 * 
 * @param pin gpio pin
 * @return gpio_direction_t 
 */
gpio_direction_t gpio_getmode(int pin);

#endif // __GPIO_H__