#ifndef __GPIO_H__
#define __GPIO_H__

typedef enum {LOW=0, HIGH=1} gpio_state;
typedef enum {OUTPUT=0, INPUT=1} gpio_direction;


/**
 * @brief Initialize GPIO
 */
void gpio_init();


/**
 * @brief Resets all gpio pins to LOW state
 */
void gpio_reset();


/**
 * @brief Read the current GPIO pin values
 * 
 * @param pin pin number
 * @return gpio_state pin value
 */
gpio_state gpio_read(int pin);


/**
 * @brief Write to a GPIO pin
 * 
 * @param pin pin number
 * @param state state
 */
void gpio_write(int pin, gpio_state state);


/**
 * @brief Set mode of a GPIO pin
 * 
 * @param pin pin number
 * @param mode mode (INPUT/OUTPUT)
 */
void gpio_setmode(int pin, gpio_direction mode);


/**
 * @brief Get mode of a GPIO pin
 * 
 * @param pin gpio pin
 * @return gpio_direction 
 */
gpio_direction gpio_getmode(int pin);

#endif // __GPIO_H__