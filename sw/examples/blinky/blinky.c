#include "../../lib/atombones.h"
#include "../../lib/stdio.h"
#include "../../lib/gpio.h"

void delay(long count)
{
    int scaling_factor = 1;
    long i=0;
    while(i++<count*scaling_factor);
    return;
}


void main()
{
    // Initialize GPIO
    gpio_init();


    // Rolling effect
    int i=0;
    int pin = 0;

    for(i=0; i<30; i++)
    {
        gpio_set((pin), !gpio_get(pin));
        
        if(pin>=GPIO_MAX_PINS-1)
            pin = 0;
        else
            pin++;

        delay(10);
    }

    gpio_reset();
    delay(100);


    // Blinking effect
    for(i=0; i<4; i++)
    {
        for(pin=0; pin<GPIO_MAX_PINS; pin++)
        {
            gpio_set((pin), !gpio_get(pin));
            delay(10);
            gpio_set((pin), !gpio_get(pin));
            delay(10);
        }
    }
    return;
}
