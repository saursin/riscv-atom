#include "../../lib/atombones.h"
#include "../../lib/stdio.h"
#include "../../lib/gpio.h"

#define DELAY_SCALING_FACTOR 10000

void delay(long count)
{
    long i=0;
    while(i++<count*DELAY_SCALING_FACTOR);
    return;
}


void main()
{
    // Initialize GPIO
    gpio_init();

    while(1)
    {
        // Effect-1: Rolling effect
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


        // Effect-2: Blinking effect
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

        gpio_reset();
        delay(100);


        // Effect-3: Disco
        for(i=0; i<4; i++)
        {
            gpio_set(0, GPIO_HIGH);
            delay(10);
            gpio_set(0, GPIO_LOW);
            delay(10);
            gpio_set(3, GPIO_HIGH);
            delay(10);
            gpio_set(3, GPIO_LOW);
            delay(10);
            gpio_set(1, GPIO_HIGH);
            delay(10);
            gpio_set(1, GPIO_LOW);
            delay(10);
            gpio_set(2, GPIO_HIGH);
            delay(10);
            gpio_set(2, GPIO_LOW);
        }

        gpio_reset();
        delay(100);

        // Effect-4: Alternate
        for(i=0; i<4; i++)
        {
            gpio_set(0, GPIO_HIGH);
            gpio_set(2, GPIO_HIGH);
            delay(20);
            gpio_set(0, GPIO_LOW);
            gpio_set(2, GPIO_LOW);
            delay(20);
            gpio_set(1, GPIO_HIGH);
            gpio_set(3, GPIO_HIGH);
            delay(20);
            gpio_set(1, GPIO_LOW);
            gpio_set(3, GPIO_LOW);
            delay(20);
        }

        gpio_reset();
        delay(100);
    }   
    return;
}
