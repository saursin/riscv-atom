#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_LOW 0
#define GPIO_HIGH 1

#define GPIO_INPUT 0
#define GPIO_OUTPUT 1


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
int gpio_read(int pin);


/**
 * @brief Write to a GPIO pin
 * 
 * @param pin pin number
 * @param state state
 */
void gpio_write(int pin, int state);


/**
 * @brief Set mode of a GPIO pin
 * 
 * @param pin pin number
 * @param mode mode (INPUT/OUTPUT)
 */
void gpio_pinmode(int pin, int mode);


#endif // __GPIO_H__