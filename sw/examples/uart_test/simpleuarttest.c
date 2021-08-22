#include "sw/lib/atombones.h"
#include "sw/lib/gpio.h"

#define UART_DIV_ADDR 0x08000000
#define UART_DAT_ADDR 0x08000004

#define DELAY_SCALING_FACTOR 10000
void delay(long count)
{
    long i=0;
    while(i++<count*DELAY_SCALING_FACTOR);
    return;
}

void sendChar(char c)
{
    *((volatile int*) UART_DAT_ADDR) = (int)c;
}

void initUART()
{
    *((volatile int*) UART_DIV_ADDR) = 1248;
}
void print(const char *p)
{
	while (*p)
    {
		sendChar(*(p++));
        delay(1);
    }
}

void main()
{
    initUART();
    while(1)
    {   
        gpio_init();

        gpio_set(0, GPIO_HIGH);
        delay(10);
        gpio_set(1, GPIO_HIGH);
        delay(10);
        gpio_set(2, GPIO_HIGH);
        delay(10);
        gpio_set(3, GPIO_HIGH);
        delay(10);
        gpio_set(4, GPIO_HIGH);
        delay(10);
        gpio_set(5, GPIO_HIGH);
        delay(10);
        gpio_set(6, GPIO_HIGH);
        delay(10);
        gpio_set(7, GPIO_HIGH);
        delay(10);
        
        const char hello[] = "Hello RISC-V!\n";
        print(hello);

        delay(5);
    }
    return;
}
