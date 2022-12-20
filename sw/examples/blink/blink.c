#include "platform.h"
#include "gpio.h"
#include "time.h"

int main()
{
    gpio_setmode(0, OUTPUT);
    while(1)
    {
        gpio_write(0, HIGH);
        sleep_ms(1000);
        gpio_write(0, LOW);
        sleep_ms(1000);
    }
    return 0;
}

