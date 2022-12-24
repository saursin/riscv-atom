#include "platform.h"
#include "mmio.h"
#include "gpio.h"

// GPIO Register Offsets
#define GPIO_REG_DAT 0x00
#define GPIO_REG_TSC 0x04
#define GPIO_REG_INT 0x08

void gpio_init()
{
    return gpio_reset();
}


void gpio_reset()
{
    // Set all pins as outputs
    REG32(GPIO_ADDR, GPIO_REG_TSC) = 0x00000000;
    // Make all pins LOW
    REG32(GPIO_ADDR, GPIO_REG_DAT) = 0x00000000;
}


gpio_state_t gpio_read(int pin)
{
    uint32_t pinvals = REG32(GPIO_ADDR, GPIO_REG_DAT);
    return bitcheck(pinvals, pin) ? HIGH : LOW;
}


uint32_t gpio_readw()
{
    return REG32(GPIO_ADDR, GPIO_REG_DAT);
}


void gpio_write(int pin, gpio_state_t state)
{
    uint32_t pinvals = REG32(GPIO_ADDR, GPIO_REG_DAT);
    pinvals = (state == HIGH) ? bitset(pinvals, pin) : bitclear(pinvals, pin);
    REG32(GPIO_ADDR, GPIO_REG_DAT) = pinvals;
}


void gpio_writew(uint32_t state)
{
    REG32(GPIO_ADDR, GPIO_REG_DAT) = state;
}


void gpio_setmode(int pin, gpio_direction_t mode)
{
    uint32_t pin_dir = REG32(GPIO_ADDR, GPIO_REG_TSC);
    pin_dir = (mode == INPUT) ? bitset(pin_dir, pin) : bitclear(pin_dir, pin);
    REG32(GPIO_ADDR, GPIO_REG_TSC) = pin_dir;
}


gpio_direction_t gpio_getmode(int pin)
{
    uint32_t pin_dir = REG32(GPIO_ADDR, GPIO_REG_TSC);
    return bitcheck(pin_dir, pin) ? INPUT : OUTPUT;
}
